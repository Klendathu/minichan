#include <gtkmm.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <json/json.h>
#include <webkit/webkit.h>
#include <pangomm/layout.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iterator>
#include <boost/regex.hpp>
#include <math.h>
#include <locale.h>
#include <gtkspell/gtkspell.h>
#include <libnotify/notify.h>

#include "ModelColumns.h"
#include "ModelThread.h"
#include "ModelPost.h"
#include "settings_dialog.h"
#include "formatting.h"
#include "json_funcs.h"

using namespace std;

//Sorry about the global variables, but using a purely funcitonal approach would be a huge PitA
string name = "Minichan";

string BOARD = "";
string LONGBOARD = "";
string THREAD = "";
bool INIT = true;
bool HIDE = false;
string curImg = "about:blank";
sigc::connection POST_CLICK_CONNECT;
sigc::connection THREAD_CLICK_CONNECT;
sigc::connection THREAD_REFRESH_CONNECT;
sigc::connection POST_REFRESH_CONNECT;
bool THREAD_REFRESH_CONNECTED;
string CHALLENGE = "";

Glib::RefPtr<Gtk::Builder> builder;

WebKitWebView* pWebKitView;

ModelColumns m_columns;
ModelThread m_thread_list;
ModelPost m_post_list;

void on_hide_toggled() {
	///Fired when the "Hide Images" menu item is toggled
	HIDE = !HIDE;
	if(HIDE) {
		webkit_web_view_load_uri(pWebKitView, "http://m.google.com");
	} else {
		webkit_web_view_load_uri(pWebKitView, curImg.c_str());
	}
}

void on_selection_changed(Glib::RefPtr<Gtk::TreeSelection> selection, WebKitWebView* webView) {
	///Fired when the selection changes in the thread list
	Gtk::TreeModel::iterator iter = selection->get_selected();
	Gtk::TreeModel::Row row = *iter;
	string tim = row[m_thread_list.tim]; //The time
	string ext = row[m_thread_list.ext]; //The file extension
	string url = "https://images.4channel.org"+BOARD+"src/"+tim+ext;
	if(BOARD=="/f/") { //This adds an exception for /f/, since it uses the file name instead of the timestamp to store files
		string fn = row[m_thread_list.fn];
		url = "https://images.4channel.org"+BOARD+"src/"+fn+ext;
	}
	if(HIDE) {
		url = "http://m.google.com";
	}
	curImg = url;
	
	const char* url_c = url.c_str();
	
	webkit_web_view_load_uri(webView, url_c);
}

void on_selection_changed_post(Glib::RefPtr<Gtk::TreeSelection> selection, WebKitWebView* webView) {
	///Fired when the selection is changed within a thread
	
	webkit_web_view_load_uri(webView, "about:blank");
	
	Gtk::TreeModel::iterator iter = selection->get_selected();
	Gtk::TreeModel::Row row = *iter;
	string tim = row[m_post_list.tim];
	string ext = row[m_post_list.ext];
	int file = row[m_post_list.file];
	
	string url = "about:blank";
	if(file) { //If there is a file...
		url = "https://images.4channel.org"+BOARD+"src/"+tim+ext; //Set the URL as per usual.
		if(BOARD=="/f/") { //But if the board is /f/...
			string fn = row[m_post_list.fn];
			url = "https://images.4channel.org"+BOARD+"src/"+fn+ext; //Use the filename instead of the time
		}
	}
	if(HIDE) {
		url = "http://m.google.com";
	}
	curImg = url;
	
	const char* url_c = url.c_str();
	webkit_web_view_load_uri(webView, url_c);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	///Write out the data retrieved by cURL
	
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
string getRemoteFile(string url_string, string filename = "") {
	///Download a file from a remote location
	///It will choose a random tempfile, unless filename is specified
	
	CURL *curl;
	FILE *fp;
	CURLcode res;
	
	const char* url = url_string.c_str();
	char * fn = tmpnam(NULL);
	
	if(!filename.empty()) {
		fn = strdup(filename.c_str()); //Duplicate filename's C string, to make it not a const
	}
	
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		
		curl_easy_cleanup(curl); //Clean up after
		fclose(fp);
		if(res == CURLE_HTTP_RETURNED_ERROR) { //This is supposed to deal with a 404 or other error, but it's a work in progress
			char * fn2 = tmpnam(NULL);
			ofstream fallback;
			fallback.open(fn2);
			fallback << "{\"error\": \"404\"}\n";
			fallback.close();
			fn = fn2;
		}
		return string(fn);
	}
	
	free(fn);
	return "";
}

Json::Value getThreads(string board) {
	///Get the list of threads as a JSON value
	
	string url_string = "https://api.4chan.org"+board+"catalog.json";
	return readFile(getRemoteFile(url_string), true);
}
Json::Value getPosts(string board, string no) {
	///Get the list of posts as a JSON value
	
	string url_string = "https://api.4chan.org"+board+"res/"+no+".json";
	return readFile(getRemoteFile(url_string), true);
}

void listView() {
	///Hide/show items for viewing the board list
	
	Gtk::ScrolledWindow *boardlist = 0;
	builder->get_widget("scrolledwindow1", boardlist);
	boardlist->set_visible(true);
	boardlist->show();
	
	gtk_widget_destroy(GTK_WIDGET(pWebKitView));
	
	Gtk::Paned *threadview = 0;
	builder->get_widget("paned1", threadview);
	threadview->set_visible(false);
	threadview->hide();
	
	Gtk::ToolItem *boardback = 0;
	builder->get_widget("toolbutton3", boardback);
	boardback->set_visible(false);
	boardback->hide();
	
	Gtk::Label *title = 0;
	builder->get_widget("label2", title);
	title->set_text("Boards");
	
	Gtk::ToolItem *refresh1 = 0;
	builder->get_widget("toolbutton5", refresh1);
	refresh1->set_visible(false);
	refresh1->hide();
	
	Gtk::ToolItem *refresh2 = 0;
	builder->get_widget("toolbutton6", refresh2);
	refresh2->set_visible(false);
	refresh2->hide();
	
	Gtk::MenuItem *item3 = 0;
	builder->get_widget("menuitem3", item3);
	item3->set_sensitive(false);

	Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(false);
	post_grid->hide();
	
	Gtk::ToggleToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->set_visible(false);
	post_but->hide();
	post_but->set_active(false);
}
void on_board_back_clicked() {
	///To be run when "back" is hit while viewing the list of threads on a board
	
	listView();
	THREAD_CLICK_CONNECT.disconnect();
}

bool refresh_captcha() {
	///Refresh the CAPTCHA in the post section
	
	string s1 = getRemoteFile("http://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc");
	
	std::ifstream t(s1.c_str());
	string s2;
	t.seekg(0, ios::end);
	s2.reserve(t.tellg());
	t.seekg(0, ios::beg);
	s2.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	
	//Google tries to be clever and make the script JSONP, so we need to remove all of the JavaScript stuff
	boost::regex jsonp1("var RecaptchaState = ");
	string s3 = boost::regex_replace(s2, jsonp1, "");
	
	boost::regex jsonp2("^\\s*([a-zA-Z0-9_]*)\\s:");
	string s4 = boost::regex_replace(s3, jsonp2, "\"$1\":");
	
	boost::regex jsonp3("^document.write\\(.*$");
	string s5 = boost::regex_replace(s4, jsonp3, "");
	
	boost::regex jsonp4(";");
	string s6 = boost::regex_replace(s5, jsonp4, "");
	
	boost::regex jsonp5("'");
	string s7 = boost::regex_replace(s6, jsonp5, "\"");
	
	Json::Value root;   // will contain the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(s7, root);
	
	Gtk::Image *captcha = 0;
	builder->get_widget("image7", captcha);

	captcha->set(getRemoteFile("http://www.google.com/recaptcha/api/image?c="+root["challenge"].asString()));
	CHALLENGE = root["challenge"].asString();
	
	return true;
}

void boardView(bool destroy=true) {
	///Show/hide widgets for viewing all of the threads on a board
	
	Gtk::ScrolledWindow *boardlist = 0;
	builder->get_widget("scrolledwindow1", boardlist);
	boardlist->set_visible(false);
	boardlist->hide();
	
	Gtk::Paned *threadview = 0;
	builder->get_widget("paned1", threadview);
	threadview->set_visible(true);
	threadview->show();
	
	if(INIT) { //To be run only the first time a board is viewed
		Gtk::Window *window = 0;
		builder->get_widget("window1", window);
		int w = 0;
		int h = 0;
		window->get_size(w, h);
		threadview->set_position((1-2/(1+sqrt(5)))*w);
		
		Gtk::TextView *view = 0;
		builder->get_widget("textview1", view);
		GtkSpellChecker* spell = gtk_spell_checker_new ();
		gtk_spell_checker_set_language (spell, setlocale(LC_ALL,NULL), NULL);
		gtk_spell_checker_attach (spell, GTK_TEXT_VIEW (view->gobj()));
		
		INIT = false;
	}
	
	Gtk::ToolItem *boardback = 0;
	builder->get_widget("toolbutton3", boardback);
	boardback->set_visible(true);
	boardback->show();
	
	Gtk::ToolItem *threadback = 0;
	builder->get_widget("toolbutton4", threadback);
	threadback->set_visible(false);
	threadback->hide();
	
	Gtk::Button* button;
	builder->get_widget("button1", button);
	button->signal_clicked().connect(sigc::ptr_fun(&on_board_back_clicked));
	button->set_visible(true);
	button->show();
	
	Gtk::ScrolledWindow *threadlist = 0;
	builder->get_widget("scrolledwindow2", threadlist);
	threadlist->set_visible(true);
	threadlist->show();
	
	Gtk::ScrolledWindow *postlist = 0;
	builder->get_widget("scrolledwindow4", postlist);
	postlist->set_visible(false);
	postlist->hide();
	
	Gtk::ScrolledWindow *webview = 0;
	builder->get_widget("scrolledwindow3", webview);
	webview->set_visible(true);
	webview->show();
	
	Gtk::ToolButton* refresh;
	builder->get_widget("toolbutton5", refresh);
	refresh->set_visible(true);
	refresh->show();
	
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	
	Gtk::Label *title = 0;
	builder->get_widget("label2", title);
	title->set_text(BOARD+" - "+LONGBOARD);
	
	Gtk::ToolButton *refresh1 = 0;
	builder->get_widget("toolbutton5", refresh1);
	refresh1->set_visible(true);
	refresh1->show();
	
	Gtk::ToolItem *refresh2 = 0;
	builder->get_widget("toolbutton6", refresh2);
	refresh2->set_visible(false);
	refresh2->hide();
	
	Gtk::MenuItem *item3 = 0;
	builder->get_widget("menuitem3", item3);
	item3->set_sensitive(false);
	
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->set_visible(true);
	post_but->show();
}

void on_thread_back_clicked() {
	///To be run when the back button is clicked while viewing a thread
	
	boardView();
	Gtk::TreeView *threadlist = 0;
	pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	imgview->set_visible(true);
	imgview->show();
	
	gtk_container_add (GTK_CONTAINER (imgview->gobj()), GTK_WIDGET (pWebKitView));
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	webkit_web_view_set_zoom_level(pWebKitView, true);
	gtk_widget_set_can_focus(GTK_WIDGET(pWebKitView), true);
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	builder->get_widget("treeview2", threadlist);
	Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
	thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
	
	POST_CLICK_CONNECT.disconnect();
	POST_REFRESH_CONNECT.disconnect();
}

void threadView() {
	///Hide or show widgets for viewing a thread
	
	Gtk::ScrolledWindow *boardlist = 0;
	builder->get_widget("scrolledwindow1", boardlist);
	boardlist->set_visible(false);
	boardlist->hide();
	
	Gtk::Paned *threadview = 0;
	builder->get_widget("paned1", threadview);
	threadview->set_visible(true);
	threadview->show();
	
	Gtk::ToolItem *boardback = 0;
	builder->get_widget("toolbutton4", boardback);
	boardback->set_visible(true);
	boardback->show();
	
	Gtk::ToolItem *listback = 0;
	builder->get_widget("toolbutton3", listback);
	listback->set_visible(false);
	listback->hide();
	
	Gtk::Button* button;
	builder->get_widget("button2", button);
	button->signal_clicked().connect(sigc::ptr_fun(&on_thread_back_clicked));
	button->set_visible(true);
	button->show();
	
	Gtk::ScrolledWindow *threadlist = 0;
	builder->get_widget("scrolledwindow2", threadlist);
	threadlist->set_visible(false);
	threadlist->hide();
	
	Gtk::ScrolledWindow *postlist = 0;
	builder->get_widget("scrolledwindow4", postlist);
	postlist->set_visible(true);
	postlist->show();
	postlist->get_vadjustment()->set_value(0);
	
	Gtk::Label *title = 0;
	builder->get_widget("label2", title);
	title->set_text("#"+THREAD+" - "+BOARD+" - "+LONGBOARD);
	
	Gtk::ToolItem *refresh1 = 0;
	builder->get_widget("toolbutton5", refresh1);
	refresh1->set_visible(false);
	refresh1->hide();
	
	Gtk::ToolItem *refresh2 = 0;
	builder->get_widget("toolbutton6", refresh2);
	refresh2->set_visible(true);
	refresh2->show();
	
	Gtk::MenuItem *item3 = 0;
	builder->get_widget("menuitem3", item3);
	item3->set_sensitive(true);
	
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->set_visible(true);
	post_but->show();
}

void getThreadData(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///Get the thread data as JSON, and then push it onto the ListStore given in m_threads_tree_model
	
	Gtk::Spinner *spin = 0;
	builder->get_widget("spinner1", spin);
	spin->set_visible(true);
	spin->show();
	
	Gtk::TreeView *tree = 0;
	builder->get_widget("treeview2", tree);
	tree->hide();
	tree->set_visible(false);
	Json::Value json = getThreads(BOARD);
	
	for(int j=0;j<json.size();++j) {
		Json::Value threads = json[j]["threads"];
		for(int i=0;i<threads.size();++i) {
			Gtk::TreeModel::Row row = *(m_threads_tree_model->append());
			
			ostringstream convert3;
			convert3 << threads[i]["replies"].asUInt();
			
			row[m_thread_list.text] = standardPostFormatting(threads[i])+"\n\n"+convert3.str()+" replies";
			row[m_thread_list.tim] = threads[i]["tim"].asString();
			row[m_thread_list.ext] = trim(threads[i]["ext"].asString());
			if(threads[i].isMember("filename")) {
				row[m_thread_list.fn] = threads[i]["filename"].asString();
			}
			
			ostringstream convert2;
			convert2 << threads[i]["no"].asInt();
			row[m_thread_list.no] = convert2.str();
		}
	}
	
	spin->hide();
	spin->set_visible(false);
	
	tree->show();
	tree->set_visible(true);
}

void getPostData(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///Get the post data as JSON, and then push it onto the ListStore given in m_threads_tree_model
	
	Json::Value json = getPosts(BOARD, THREAD);
	m_threads_tree_model->clear();
	Json::Value posts = json["posts"];
	for(int i=0;i<posts.size();++i) {
		Gtk::TreeModel::Row row = *(m_threads_tree_model->append());
		
		string text = standardPostFormatting(posts[i]);
		row[m_post_list.text] = text;
		row[m_post_list.tim] = posts[i]["tim"].asString();
		row[m_post_list.ext] = trim(posts[i]["ext"].asString());
		
		ostringstream convert2;
		convert2 << posts[i]["no"].asInt();
		row[m_post_list.no] = convert2.str();
		
		row[m_post_list.file] = posts[i].isMember("filename") ? 1 : 0;
		
		if(posts[i].isMember("filename")) {
			row[m_post_list.fn] = posts[i]["filename"].asString();
		}
		
		boost::regex double_meme_arrows("(&gt;&gt;(?!&gt;)\\d{1,})");
		string curPost = stripBreaks(posts[i]["com"].asString());
		boost::sregex_token_iterator iter(curPost.begin(), curPost.end(), double_meme_arrows, 0);
		boost::sregex_token_iterator end;
		
		boost::regex actual_arrows("&gt;&gt;");
		string tool = "";
		for( ; iter != end; ++iter ) {
			string s = boost::regex_replace(string(*iter), actual_arrows, "");
			int s2 = atoi(s.c_str());
			for(int j=0;j<posts.size();++j) {
				if(posts[j]["no"].asInt() == s2) {
					tool = tool+"<b>"+*iter+"</b>\n"+stripTags(posts[j]["com"].asString())+"\n\n";
					break;
				}
			}
		}
		
		row[m_post_list.tool] = trim(tool);
	}
}

void on_refresh_threads_clicked(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///To be executed when the refresh button is clicked while viewing the thread list
	
	m_threads_tree_model->clear();
	getThreadData(m_threads_tree_model);
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	
	Gtk::ScrolledWindow *scroll = 0;
	builder->get_widget("scrolledwindow2", scroll);
	scroll->get_vadjustment()->set_value(0);
}

void on_refresh_posts_clicked(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///To be executed when the refresh button is clicked while viewing a thread
	
	Gtk::Spinner *spin = 0;
	builder->get_widget("spinner1", spin);
	spin->show();
	spin->set_visible(true);
	
	Gtk::TreeView *tree = 0;
	builder->get_widget("treeview4", tree);
	tree->hide();
	tree->set_visible(false);
	
	m_threads_tree_model->clear();
	getPostData(m_threads_tree_model);
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	
	spin->hide();
	spin->set_visible(false);
	
	tree->show();
	tree->set_visible(true);
}

void on_post_clicked() {
	///To be executed when the posting form is submitted
	
	CURL *curl;
	FILE *fp;
	CURLcode res;
	string url_string = "https://sys.4chan.org"+BOARD+"post";
	const char* url = url_string.c_str();
	
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	struct curl_slist *headerlist=NULL;
	static const char buf[] = "Expect:";
	
	curl_global_init(CURL_GLOBAL_ALL);
	
	Gtk::TextView *com_box = 0;
	builder->get_widget("textview1", com_box);
	string com = com_box->get_buffer()->get_text();
	
	Gtk::Entry *captcha = 0;
	builder->get_widget("entry2", captcha);
	string captcha_text = captcha->get_text();
	
	cout << com << endl;
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "com",
		CURLFORM_COPYCONTENTS, com.c_str(),
		CURLFORM_END);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "recaptcha_response_field",
		CURLFORM_COPYCONTENTS, captcha_text.c_str(),
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "recaptcha_challenge_field",
		CURLFORM_COPYCONTENTS, CHALLENGE.c_str(),
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "pwd",
		CURLFORM_COPYCONTENTS, "999999", //Just a filler value. TODO
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "MAX_FILE_SIZE",
		CURLFORM_COPYCONTENTS, "3145728",
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "mode",
		CURLFORM_COPYCONTENTS, "regist",
		CURLFORM_END);
	
	if(THREAD != "") {
		//Only run if there is no current thread
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "resto",
			CURLFORM_COPYCONTENTS, THREAD.c_str(),
			CURLFORM_END);
	}
	
	Gtk::FileChooserButton *file = 0;
	builder->get_widget("filechooserbutton1", file);
	string file_name = file->get_filename();
	
	if((file_name != "") && (THREAD != "")) {
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "upfile",
			CURLFORM_FILE, file_name.c_str(),
			CURLFORM_END);
	}
			
	curl = curl_easy_init();
	headerlist = curl_slist_append(headerlist, buf);
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		char * fn = tmpnam(NULL);
		fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		
		res = curl_easy_perform(curl);
		
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
		
		/* then cleanup the formpost chain */ 
		curl_formfree(formpost);
		
		/* free slist */ 
		curl_slist_free_all (headerlist);
		
		std::ifstream t(fn);
		std::string str;

		t.seekg(0, std::ios::end);   
		str.reserve(t.tellg());
		t.seekg(0, std::ios::beg);
	
		string retval = str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		} else {
			cout << str << endl;
			boost::regex success("successful", boost::regex::icase);
			if(boost::regex_search(str, success)) {
				cout << "Clear all text boxes" << endl;
				
				com_box->get_buffer()->set_text("");
				captcha->set_text("");
				file->unselect_all();
				
				Gtk::ToggleToolButton *post_but;
				builder->get_widget("toolbutton8", post_but);
				post_but->set_active(false);
				
				Gtk::ToolButton *ref_but;
				builder->get_widget("toolbutton6", ref_but);
				ref_but->signal_clicked();
				
				notify_init ("Minichan");
				NotifyNotification * Hello = notify_notification_new ("Post successful", "", "dialog-information");
				notify_notification_show (Hello, NULL);
			} else {
				cout << "Post failed!" << endl;
				refresh_captcha();
				
				notify_init ("Minichan");
				NotifyNotification * Hello = notify_notification_new ("Post failed", "", "dialog-information");
				notify_notification_show (Hello, NULL);
			}
		}
	}
}

bool on_key_press_or_release_event(GdkEventKey* event, Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///To be executed when a key is pressed or released
	
	if (event->type == GDK_KEY_PRESS && event->keyval == GDK_KEY_F5) {
		on_refresh_threads_clicked(m_threads_tree_model);
		return true;
	}
	return false;
}
bool on_key_press_thread(GdkEventKey* event, Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	///To be run when a key is pressed or released. TODO: Check why there are two of these functions
	
	if (event->type == GDK_KEY_PRESS && event->keyval == GDK_KEY_F5) {
		on_refresh_posts_clicked(m_threads_tree_model);
		return true;
	}
	return false;
}

void open_thread_in_browser() {
	///Open a thread in the default web browser
	
	string url = "https://boards.4chan.org"+BOARD+"res/"+THREAD;
	GError *error = NULL;
	gtk_show_uri(gdk_screen_get_default(), url.c_str(), gtk_get_current_event_time(), &error);
}

void on_post_activated1(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model, Glib::RefPtr<Gtk::ListStore> store) {
	///To be run when a post is double clicked
	
	Gtk::ToggleToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	
	if(!post_but->get_active()) {
		string tool = row[m_post_list.tool];
		string no = row[m_post_list.no];
		string com = row[m_post_list.text];
		
		Gtk::Window *window = 0;
		builder->get_widget("window1", window);
		
		boost::regex r1("^<b>.*</b> #\\d*$");
		string s1 = boost::regex_replace(com, r1, "");
		
		Gtk::MessageDialog dialog(*window, "");
		dialog.set_secondary_text(tool+((tool != "") ? "\n\n" : "")+"<i>"+addLinks(trim(stripTags(com)))+"</i>", true);
		
		dialog.run();
	} else {
		Gtk::TextView *com_box = 0;
		builder->get_widget("textview1", com_box);
		string com = com_box->get_buffer()->get_text();
		string no = row[m_post_list.no];
		com_box->get_buffer()->set_text(com+">>"+no);
	}
}

void on_thread_activated(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model, Glib::RefPtr<Gtk::ListStore> store) {
	///To be run when a thread is double clicked
	
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	string no = row[m_thread_list.no];
	
	THREAD = no;
	
	cout << THREAD << endl;
	cout << BOARD << endl;
	
	threadView();
	
	Gtk::TreeView *postlist = 0;
	builder->get_widget("treeview4", postlist);
	postlist->set_visible(true);
	postlist->show();
	
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	
	gtk_container_add (GTK_CONTAINER (imgview->gobj()), GTK_WIDGET (pWebKitView));
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	webkit_web_view_set_zoom_level(pWebKitView, true);
	gtk_widget_set_can_focus(GTK_WIDGET(pWebKitView), true);
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	
	Glib::RefPtr<Gtk::ListStore> m_posts_tree_model;
	m_posts_tree_model = Gtk::ListStore::create(m_post_list);
	m_posts_tree_model->clear();
	postlist->set_model(m_posts_tree_model);
	postlist->set_search_column(1);
	postlist->get_column(1)->set_visible(false);
	postlist->get_column(2)->set_visible(false);
	postlist->get_column(3)->set_visible(false);
	postlist->get_column(4)->set_visible(false);
	postlist->get_column(5)->set_visible(false);
	
	postlist->signal_key_press_event().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_key_press_thread), m_posts_tree_model));
	postlist->signal_key_release_event().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_key_press_thread), m_posts_tree_model));
	postlist->add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	
	Gtk::CellRendererText* cellRender = dynamic_cast<Gtk::CellRendererText*>(postlist->get_column_cell_renderer(0));
	cellRender->property_wrap_mode() = Pango::WRAP_WORD;
	cellRender->property_wrap_width() = 400;
	cellRender->set_alignment(0, 0);
	
	Glib::RefPtr<Gtk::TreeSelection> thread_selection = postlist->get_selection();
	thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed_post), thread_selection, pWebKitView));
	
	getPostData(m_posts_tree_model);
	
	postlist->grab_focus();
	
	Gtk::ToolButton* refresh;
	builder->get_widget("toolbutton6", refresh);
	refresh->set_visible(true);
	refresh->show();
	POST_REFRESH_CONNECT = refresh->signal_clicked().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_refresh_posts_clicked), m_posts_tree_model));
	
	Gtk::ScrolledWindow *postwin = 0;
	builder->get_widget("scrolledwindow4", postwin);
	postwin->set_visible(true);
	postwin->show();
	
	POST_CLICK_CONNECT = postlist->signal_row_activated().connect(sigc::bind<Glib::RefPtr<Gtk::TreeModel>, Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_post_activated1), postlist->get_model(), m_posts_tree_model));
}

void on_button_clicked(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model) {
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	string shortname = row[m_columns.name];
	string longname = row[m_columns.longname];
	
	BOARD = shortname;
	LONGBOARD = longname;
	
	boardView();
	
	Gtk::TreeView *threadlist = 0;
	builder->get_widget("treeview2", threadlist);
	
	pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	imgview->set_visible(true);
	imgview->show();
	
	gtk_container_add (GTK_CONTAINER (imgview->gobj()), GTK_WIDGET (pWebKitView));
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	webkit_web_view_set_zoom_level(pWebKitView, true);
	gtk_widget_set_can_focus(GTK_WIDGET(pWebKitView), true);
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	
	Glib::RefPtr<Gtk::ListStore> m_threads_tree_model;
	m_threads_tree_model = Gtk::ListStore::create(m_thread_list);
	m_threads_tree_model->clear();
	threadlist->set_model(m_threads_tree_model);
	threadlist->set_search_column(1);
	threadlist->get_column(1)->set_visible(false);
	threadlist->get_column(2)->set_visible(false);
	threadlist->get_column(3)->set_visible(false);
	threadlist->get_column(4)->set_visible(false);
	
	threadlist->signal_key_press_event().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_key_press_or_release_event), m_threads_tree_model));
	threadlist->signal_key_release_event().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_key_press_or_release_event), m_threads_tree_model));
	threadlist->add_events(Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK);
	
	Gtk::CellRendererText* cellRender = dynamic_cast<Gtk::CellRendererText*>(threadlist->get_column_cell_renderer(0));
	cellRender->property_wrap_mode() = Pango::WRAP_WORD;
	cellRender->property_wrap_width() = 400;
	cellRender->set_alignment(0, 0);
	
	Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
	thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
	
	getThreadData(m_threads_tree_model);
	
	threadlist->grab_focus();
	
	Gtk::ToolButton* refresh;
	builder->get_widget("toolbutton5", refresh);
	refresh->set_visible(true);
	refresh->show();
	if(!THREAD_REFRESH_CONNECTED) {
		THREAD_REFRESH_CONNECT = refresh->signal_clicked().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_refresh_threads_clicked), m_threads_tree_model));
	}
	
	THREAD_CLICK_CONNECT = threadlist->signal_row_activated().connect(sigc::bind<Glib::RefPtr<Gtk::TreeModel>, Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_thread_activated), threadlist->get_model(), m_threads_tree_model));
}

void saveImageAs() {
	///Download the image of the currently selected post, with a file save dialog and everything
	
	Gtk::TreeView *treeview = 0;
	builder->get_widget("treeview4", treeview);
	
	Glib::RefPtr<Gtk::TreeSelection> tree_selection = treeview->get_selection();
	Glib::RefPtr<Gtk::TreeModel> model = treeview->get_model();
	
	Gtk::TreeModel::iterator iter = tree_selection->get_selected(model);
	Gtk::TreeModel::Row row = *iter;
	
	string tim = row[m_post_list.tim];
	string ext = row[m_thread_list.ext];
	int file = row[m_post_list.file];

	string url = "about:blank";
	if(file) {
		url = "https://images.4channel.org"+BOARD+"src/"+tim+ext;
		cout << "\n\n\n\n\n\n" << url << "\n\n\n\n\n\n" << endl;
		
		Gtk::FileChooserDialog dialog("Save image as...", Gtk::FILE_CHOOSER_ACTION_SAVE);
		
		dialog.set_current_name(tim+ext);
		
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

		int result = dialog.run();
		switch(result) {
			case(Gtk::RESPONSE_OK):
			{
				string fn = dialog.get_filename();
				getRemoteFile(url, fn);
			}
		}
	}
}

void show_post_section() {
	///Show the post form
	
	Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(true);
	post_grid->show();
}
void hide_post_section() {
	///Hide the post form
	
	Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(false);
	post_grid->hide();
}

void on_post_button_toggled() {
	///To be run when the posting button is toggled
	
	Gtk::ToggleToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	if(!post_but->get_active()) {
		hide_post_section();
	} else {
		show_post_section();
		refresh_captcha();
	}
}

void startup(Glib::RefPtr<Gtk::Application> app) {
	///To be run at startup
	
	GtkSettings *settings;
	settings = gtk_settings_get_default();
	g_object_set (G_OBJECT (settings), "gtk-application-prefer-dark-theme", FALSE, NULL);
	
	Gtk::Main kit;
	
	builder = Gtk::Builder::create_from_file(DATADIR "/main.glade");
	Gtk::Window *window = 0;
	builder->get_widget("window1", window);
	window->set_wmclass("org.gtkmm.examples.application", name);
	window->maximize();
	
	Gtk::TreeView *listview = 0;
	builder->get_widget("treeview1", listview);
	
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
	m_refTreeModel = Gtk::ListStore::create(m_columns);
	listview->set_model(m_refTreeModel);
	listview->set_search_column(1);
	
	Glib::RefPtr<Gtk::TreeSelection> list_selection = listview->get_selection();
	listview->signal_row_activated().connect(sigc::bind<Glib::RefPtr<Gtk::TreeModel> >(sigc::ptr_fun(&on_button_clicked), listview->get_model()));
	
	Json::Value json = readFile(getRemoteFile("https://api.4chan.org/boards.json"));
	Json::Value boards = json["boards"];
	for(int i=0;i<boards.size();++i) {
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_columns.name] = "/"+boards[i]["board"].asString()+"/";
		row[m_columns.longname] = boards[i]["title"].asString();
	}
	listview->set_rules_hint(true);
	
	Gtk::Menu *mainmenu = 0;
	builder->get_widget("menu1", mainmenu);
	Gtk::MenuToolButton *toolbut = 0;
	builder->get_widget("toolbutton2", toolbut);
	toolbut->set_menu(*mainmenu);
	
	Gtk::MenuItem *item1 = 0;
	builder->get_widget("menuitem1", item1);
	item1->signal_activate().connect(sigc::ptr_fun(&nameSettingsDialog));
	
	Gtk::CheckMenuItem *hideimages = 0;
	builder->get_widget("menuitem2", hideimages);
	hideimages->signal_toggled().connect(sigc::ptr_fun(&on_hide_toggled));
	
	Gtk::MenuItem *item3 = 0;
	builder->get_widget("menuitem3", item3);
	item3->signal_activate().connect(sigc::ptr_fun(&open_thread_in_browser));
	
	Gtk::MenuItem *d_image = 0;
	builder->get_widget("menuitem5", d_image);
	d_image->signal_activate().connect(sigc::ptr_fun(&saveImageAs));
	
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->signal_clicked().connect(sigc::ptr_fun(&on_post_button_toggled));
	
	Gtk::Button *post_but2 = 0;
	builder->get_widget("button4", post_but2);
	post_but2->signal_clicked().connect(sigc::ptr_fun(&on_post_clicked));
	
	kit.run(*window);
}

int main(int argc, char *argv[]) {
	//Gtk::Main kit(argc, argv);
	
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create("org.gtkmm.examples.application");
	app->signal_startup().connect(sigc::bind< Glib::RefPtr<Gtk::Application> >(sigc::ptr_fun(&startup), app));
	
	app->run(argc, argv);
	return 0;
}

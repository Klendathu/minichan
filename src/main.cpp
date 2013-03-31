#include <gtkmm.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <json/json.h>
//#include <giomm.h>
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

using namespace std;
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

class ModelColumns : public Gtk::TreeModel::ColumnRecord {
	public:

		ModelColumns()
		{ add(name); add(longname);}

		Gtk::TreeModelColumn<string> name;
		Gtk::TreeModelColumn<string> longname;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pixbuf;
};
ModelColumns m_columns;

class ModelThread : public Gtk::TreeModel::ColumnRecord {
	public:

		/*ModelThread()
		{ add(m_col_text); add(m_col_num); add(m_col_ext);}

		Gtk::TreeModelColumn<string> m_col_text;
		Gtk::TreeModelColumn<string> m_col_num;
		Gtk::TreeModelColumn<string> m_col_ext;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pixbuf;*/
		ModelThread()
		{ add(text); add(tim); add(ext); add(no); add(fn);}

		Gtk::TreeModelColumn<string> text;
		Gtk::TreeModelColumn<string> tim;
		Gtk::TreeModelColumn<string> ext;
		Gtk::TreeModelColumn<string> no;
		Gtk::TreeModelColumn<string> fn;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > thumb;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pixbuf;
};
class ModelPost : public Gtk::TreeModel::ColumnRecord {
	public:

		/*ModelThread()
		{ add(m_col_text); add(m_col_num); add(m_col_ext);}

		Gtk::TreeModelColumn<string> m_col_text;
		Gtk::TreeModelColumn<string> m_col_num;
		Gtk::TreeModelColumn<string> m_col_ext;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pixbuf;*/
		ModelPost()
		{ add(text); add(tim); add(ext); add(no); add(file); add(tool); add(fn);}

		Gtk::TreeModelColumn<string> text;
		Gtk::TreeModelColumn<string> tim;
		Gtk::TreeModelColumn<string> ext;
		Gtk::TreeModelColumn<string> no;
		Gtk::TreeModelColumn<int> file;
		Gtk::TreeModelColumn<string> tool;
		Gtk::TreeModelColumn<string> fn;
		//Gtk::TreeModelColumn<Gdk::Pixbuf> thumb;
		//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_col_pixbuf;
};
ModelThread m_thread_list;
ModelPost m_post_list;

Json::Value readFile(string path, bool timFix=false) {
	std::ifstream t(path.c_str());
	std::string str;

	t.seekg(0, std::ios::end);   
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	string s = str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	if(timFix) {
		boost::regex tim("\"tim\":(\\d{13})");
		string s2 = boost::regex_replace(s, tim, "\"tim\":\"$1\"");
		s = s2;
	}
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(s, root);
	if ( !parsingSuccessful ) {
		// report to the user the failure and their locations in the document.
		//cerr  << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
		return 1;
	}
	return root;
}

void on_hide_toggled() {
	HIDE = !HIDE;
	if(HIDE) {
		webkit_web_view_load_uri(pWebKitView, "http://m.google.com");
	} else {
		webkit_web_view_load_uri(pWebKitView, curImg.c_str());
	}
}

void on_selection_changed(Glib::RefPtr<Gtk::TreeSelection> selection, WebKitWebView* webView) {
	//cout << "yolo" << endl;
	Gtk::TreeModel::iterator iter = selection->get_selected();
	Gtk::TreeModel::Row row = *iter;
	string tim = row[m_thread_list.tim];
	string ext = row[m_thread_list.ext];
	string url = "https://images.4channel.org"+BOARD+"src/"+tim+ext;
	if(BOARD=="/f/") {
		cout << "Currently on /f/" << endl;
		string fn = row[m_thread_list.fn];
		url = "https://images.4channel.org"+BOARD+"src/"+fn+ext;
		cout << url << endl;
	}
	if(HIDE) {
		url = "http://m.google.com";
	}
	curImg = url;
	//cout << "[" << url << "]" << endl;
	const char* url_c = url.c_str();
	
	//Gtk::CheckMenuItem *hideimages = 0;
	//builder->get_widget("menuitem2", hideimages);
	/*if(!HIDE) {
		webkit_web_view_load_uri(webView, url_c);
	} else {
		webkit_web_view_load_uri(webView, "http://www.google.com");
	}*/
	webkit_web_view_load_uri(webView, url_c);
	//webkit_web_view_execute_script(pWebKitView, "document.body.style.display='table'; document.querySelector('img').style.display='table-cell'; document.querySelector('img').style.verticalAlign='center';");
	//cout << "'" << model_value << "'" << endl;
	//cout << "\n\n\n\n\n\n" << url << "\n\n\n\n\n\n" << endl;
}

void on_selection_changed_post(Glib::RefPtr<Gtk::TreeSelection> selection, WebKitWebView* webView) {
	webkit_web_view_load_uri(webView, "about:blank");
	//cout << "swag" << endl;
	Gtk::TreeModel::iterator iter = selection->get_selected();
	Gtk::TreeModel::Row row = *iter;
	string tim = row[m_post_list.tim];
	string ext = row[m_post_list.ext];
	int file = row[m_post_list.file];
	//cout << "[" << fn << "]" << endl;
	string url = "about:blank";
	if(file) {
		url = "https://images.4channel.org"+BOARD+"src/"+tim+ext;
		if(BOARD=="/f/") {
			string fn = row[m_post_list.fn];
			url = "https://images.4channel.org"+BOARD+"src/"+fn+ext;
		}
	}
	if(HIDE) {
		url = "http://m.google.com";
	}
	curImg = url;
	//cout << "[" << url << "]" << endl;
	const char* url_c = url.c_str();
	webkit_web_view_load_uri(webView, url_c);
	//webkit_web_view_execute_script(pWebKitView, "document.body.style.display='table'; document.querySelector('img').style.display='table-cell'; document.querySelector('img').style.verticalAlign='center';");
	//cout << "'" << model_value << "'" << endl;
	//cout << "\n\n\n\n\n\n{" << url << "}\n\n\n\n\n\n" << endl;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);
	return written;
}
string getRemoteFile(string url_string, string filename = "") {
	CURL *curl;
	FILE *fp;
	CURLcode res;
	//string url = "https://api.4chan.org/"+board+"/catalog.json";
	const char* url = url_string.c_str();
	char * fn = tmpnam(NULL);
	
	if(!filename.empty()) {
		fn = strdup(filename.c_str());
		//free(cp);
		//char * fn = filename;
	}
	//char outfilename[102400] = fn;
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
		if(res == CURLE_HTTP_RETURNED_ERROR) {
			char * fn2 = tmpnam(NULL);
			ofstream fallback;
			fallback.open(fn2);
			fallback << "{\"error\": \"404\"}\n";
			fallback.close();
			fn = fn2;
		}
		return string(fn);
	}
	//ofstream fallback;
	//fallback.open(fn);
	//fallback << "{\"error\": \"404\"}\n";
	//fallback.close();
	
	free(fn);
	return "";
	//return string(fn);
}

Json::Value getThreads(string board) {
	/*CURL *curl;
	FILE *fp;
	CURLcode res;
	//string url = "https://api.4chan.org/"+board+"/catalog.json";
	string url_string = "https://api.4chan.org"+board+"catalog.json";
	const char* url = url_string.c_str();
	char * fn = tmpnam(NULL);
	//char outfilename[102400] = fn;
	curl = curl_easy_init();
	if (curl) {
		fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		/ always cleanup *
		curl_easy_cleanup(curl);
		fclose(fp);
		return readFile(string(fn));
	}
	return -1;*/
	string url_string = "https://api.4chan.org"+board+"catalog.json";
	return readFile(getRemoteFile(url_string), true);
}
Json::Value getPosts(string board, string no) {
	string url_string = "https://api.4chan.org"+board+"res/"+no+".json";
	return readFile(getRemoteFile(url_string), true);
}

string stripTags(string s) {
	boost::regex br("<br?>");
	string s2 = boost::regex_replace(s, br, "\n");
	
	boost::regex html("<(.|\n)*?>");
	string s3 = boost::regex_replace(s2, html, "");
	
	return s3;
}

string stripBreaks(string s) {
  //int br;
  /*while((br = s.find("<br>")) != string::npos)
	s.replace(br, strlen("<br>"), "\n");
  while((br = s.find("<br />")) != string::npos)
	//s.erase(br, strlen("<br />"));
	s.replace(br, strlen("<br />"), "\n");
  *while((br = s.find("<wbr>")) != string::npos)
	s.replace(br, strlen("<wbr>"), "\n");
  while((br = s.find("class=\"quote\"")) != string::npos)
	s.erase(br, strlen("class=\"quote\""));
  //cout << s << endl;*/
	/*cout << s << endl;
	string empty = "duple";
	//regex self_regex("/<(.|\n)*?>/", regex_constants::ECMAScript | regex_constants::icase);
	regex self_regex("<(.|\n)*?>");
	regex regex2("b");
	string s2;
	regex_replace(back_inserter(s2), s.begin(), s.end(), regex2, string("herro"));
	//cout << "\n" << s2 << "\n" << "----------" << endl;*/
	//boost::regex br("<br?>");
	//string s2 = boost::regex_replace(s, br, "\n");
	
	//boost::regex link1("<a href=\".*?\".*?>(.*?)</a>");
	//string s3 = boost::regex_replace(s2, link1, "[$1]");
	
	//boost::regex html("<(.|\n)*?>");
	//boost::regex meme_arrows("^((\\s)?>.*?)$");
	//boost::regex meme_arrows("^(>.*?)$");
	//boost::regex meme_arrows("^\\s?(&gt;.*?)$");
	//boost::regex meme_arrows("^\\s?(&gt;[^(&gt;)]*)$");
	boost::regex meme_arrows("^\\s?(&gt;(?!&gt;).*?)$");
	//boost::regex double_meme_arrows("^\\s?(&gt;&gt;\\d*)$");
	boost::regex double_meme_arrows("(&gt;&gt;(?!&gt;)\\d{1,})");
	boost::regex triple_meme_arrows("^\\s(&gt;&gt;&gt;.*?)$");
	
	//return argc > 1 ? !boost::regex_match(argv[1], e) : 2;
	//string s3 = boost::regex_replace(s2, html, "");
	string s3 = stripTags(s);
	//string s4 = boost::regex_replace(s3, meme_arrows, "<span foreground='forestGreen'>$1</span>");
	string s4 = boost::regex_replace(s3, meme_arrows, "<i>$1</i>");
	string s5 = boost::regex_replace(s4, double_meme_arrows, "<span foreground='purple'>$1</span>");
	string s6 = boost::regex_replace(s5, triple_meme_arrows, "<span foreground='red'>$1</span>");
	
	boost::regex urls("((https?|ftp|file)://[-A-Z0-9+&@#/%?=~_|!:,.;]*[A-Z0-9+&@#/%=~_|])", boost::regex::icase);
	string s7 = boost::regex_replace(s5, urls, "<span foreground='blue'>$1</span>");
	//string s6 = boost::regex_replace(s5, urls, "<a href='$1'>$1</a>");
	
	return s7;
}


void listView() {
	//BOARD = "";
	
	Gtk::ScrolledWindow *boardlist = 0;
	builder->get_widget("scrolledwindow1", boardlist);
	boardlist->set_visible(true);
	boardlist->show();
	
	//Gtk::ScrolledWindow *webview = 0;
	//builder->get_widget("scrolledwindow3", webview);
	//webview->remove(pWebKitView);
	gtk_widget_destroy(GTK_WIDGET(pWebKitView));
	
	Gtk::Paned *threadview = 0;
	builder->get_widget("paned1", threadview);
	threadview->set_visible(false);
	threadview->hide();
	
	Gtk::ToolItem *boardback = 0;
	builder->get_widget("toolbutton3", boardback);
	boardback->set_visible(false);
	boardback->hide();
	
	/*Gtk::Button* refresh;
	builder->get_widget("button3", refresh);
	refresh->set_visible(false);
	refresh->hide();*/
	
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
	
	//THREAD = "";
}
void on_board_back_clicked() {
	listView();
	THREAD_CLICK_CONNECT.disconnect();
}

bool refresh_captcha() {
	string s1 = getRemoteFile("http://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc");
	//fp = fopen(fn,"rb");
	/*ifstream myReadFile;
	myReadFile.open(s1);
	char output[500];
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			//cout<<output;
		}
	}*/
	std::ifstream t(s1.c_str());
	string s2;
	t.seekg(0, ios::end);
	s2.reserve(t.tellg());
	t.seekg(0, ios::beg);
	s2.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	
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
	
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(s7, root);
	
	//cout << "[" << root.toStyledString() << "]" << endl;
	
	//cout << root["challenge"] << endl;
	
	//Json::Value challengePage = readFile(getRemoteFile("http://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc"));
	//cout << challengePage.toStyledString() << endl;
	Gtk::Image *captcha = 0;
	builder->get_widget("image7", captcha);
	//captcha->set(getRemoteFile("http://www.google.com/recaptcha/api/image?c=03AHJ_VusYp6oIw56geKjgWfqSPyu_iyx1hf871VIWDnBVExgn0mhTjfSpwuTUI3B1sRtR54klJs2F13h5N307LmczsGp7qSvhJM2DL5EoK2sREsQnrX_INP60X9YH3pO0h5D5mRUignhf_QPFVmEnH08yi_zwwrQG8HifUWrvDKpcDNm1EfcMw0Q"));
	captcha->set(getRemoteFile("http://www.google.com/recaptcha/api/image?c="+root["challenge"].asString()));
	CHALLENGE = root["challenge"].asString();
	
	return true;
}

void boardView(bool destroy=true) {
	//cout << "\n\n\n\n\n\n%%%" << BOARD << "%%%\n\n\n\n\n\n" << endl;
	
	Gtk::ScrolledWindow *boardlist = 0;
	builder->get_widget("scrolledwindow1", boardlist);
	boardlist->set_visible(false);
	boardlist->hide();
	
	Gtk::Paned *threadview = 0;
	builder->get_widget("paned1", threadview);
	threadview->set_visible(true);
	threadview->show();
	
	if(INIT) {
		Gtk::Window *window = 0;
		builder->get_widget("window1", window);
		int w = 0;
		int h = 0;
		window->get_size(w, h);
		//threadview->set_position((1-1/1.68)*w);
		//threadview->set_position((1-1/((1+sqrt(5))/2))*w);
		threadview->set_position((1-2/(1+sqrt(5)))*w);
		//cout << ((1-1/1.68)*w) << endl;
		//Gtk::Button *post_but = 0;
		//builder->get_widget("button4", post_but);
		//post_but->signal_clicked().connect(sigc::ptr_fun(&on_post_clicked));
		
		/*Gtk::TextView *view = 0;
		builder->get_widget("textview1", view);
		GtkSpellChecker* spell = gtk_spell_checker_new ();
		gtk_spell_checker_set_language (spell, "en_US", NULL);
		gtk_spell_checker_attach (spell, GTK_TEXT_VIEW (view));*/
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
	
	/*if(destroy) {
		gtk_widget_destroy(GTK_WIDGET(pWebKitView));
	}*/
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	//cout << "\n\n\n\n\n\n###" << BOARD << "###\n\n\n\n\n\n" << endl;
	
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

	/*Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(true);
	post_grid->show();*/
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->set_visible(true);
	post_but->show();
	
	//Gtk::TreeView *treeview = 0;
	//builder->get_widget("treeview2", treeview);
	//threadview->set_visible(true);
	//threadview->show();
	//Gtk::TreeModel *m_threads_tree_model = 0;
	//m_threads_tree_model = treeview->get_model();
	
	//Gtk::ToolButton* refresh;
	//builder->get_widget("toolbutton5", refresh);
	//refresh->set_visible(true);
	//refresh->show();
	//THREAD_REFRESH_CONNECT = refresh1->signal_clicked().connect(sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_refresh_threads_clicked), *m_threads_tree_model));


}

void on_thread_back_clicked() {
	//cout << "\n\n\n\n\n\n" << pWebKitView << "\n\n\n\n\n\n" << endl;
	boardView();
	Gtk::TreeView *threadlist = 0;
	pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	imgview->set_visible(true);
	imgview->show();
	//imgview->add(webView);
	gtk_container_add (GTK_CONTAINER (imgview->gobj()), GTK_WIDGET (pWebKitView));
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	webkit_web_view_set_zoom_level(pWebKitView, true);
	gtk_widget_set_can_focus(GTK_WIDGET(pWebKitView), true);
	gtk_widget_show(GTK_WIDGET(pWebKitView));
	builder->get_widget("treeview2", threadlist);
	Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
	thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
	
	POST_CLICK_CONNECT.disconnect();
	//THREAD_REFRESH_CONNECT.disconnect();
	POST_REFRESH_CONNECT.disconnect();
	
	//THREAD = "";
	
	cout << THREAD << endl;
}

void threadView() {
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
	//button->signal_clicked().connect(sigc::bind<Gtk::TreeModel::Path path, Gtk::TreeViewColumn*, Glib::RefPtr<Gtk::TreeModel> >(sigc::ptr_fun(&on_thread_back_clicked)), path, column, model);
	button->signal_clicked().connect(sigc::ptr_fun(&on_thread_back_clicked));
	button->set_visible(true);
	button->show();
	
	//sigc::bind<Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_key_press_or_release_event), m_threads_tree_model));
	Gtk::ScrolledWindow *threadlist = 0;
	builder->get_widget("scrolledwindow2", threadlist);
	threadlist->set_visible(false);
	threadlist->hide();
	
	Gtk::ScrolledWindow *postlist = 0;
	builder->get_widget("scrolledwindow4", postlist);
	postlist->set_visible(true);
	postlist->show();
	postlist->get_vadjustment()->set_value(0);
	
	//webkit_web_view_load_uri(pWebKitView, "about:blank");
	
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

	/*Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(true);
	post_grid->show();*/
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->set_visible(true);
	post_but->show();
}

string trim(string s, const char* t = " \t\n\r\f\v")
{
	s.erase(0, s.find_first_not_of(t));
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

string standardPostFormatting(Json::Value post) {
	string txt = stripBreaks(post["com"].asString());
		
	//if(posts[i]["sub"].asString() != "") {
	if(post.isMember("sub")) {
		txt = "<u>"+post["sub"].asString()+"</u>\n"+txt;
	}/* else {
		row[m_thread_list.m_col_text] = stripBreaks(threads[i]["com"].asString());
	}*/
	ostringstream convert;
	convert << post["no"].asInt();
	txt = "<b>"+post["name"].asString()+post["trip"].asString()+"</b> #"+convert.str()+"\n"+txt;
	return txt;
}

void getThreadData(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	Gtk::Spinner *spin = 0;
	builder->get_widget("spinner1", spin);
	spin->set_visible(true);
	spin->show();
	
	Gtk::TreeView *tree = 0;
	builder->get_widget("treeview2", tree);
	tree->hide();
	tree->set_visible(false);
	Json::Value json = getThreads(BOARD);
	//cout << json << endl;
	/*if(json.isMember("error")) {
		cout << "O noes!" << endl;
	}*/
	for(int j=0;j<json.size();++j) {
		Json::Value threads = json[j]["threads"];
		for(int i=0;i<threads.size();++i) {
			Gtk::TreeModel::Row row = *(m_threads_tree_model->append());
			
			/*string txt = stripBreaks(threads[i]["com"].asString());
			
			if(threads[i]["sub"].asString() != "") {
				txt = "<u>"+threads[i]["sub"].asString()+"</u>\n"+txt;
			}/ else {
				row[m_thread_list.m_col_text] = stripBreaks(threads[i]["com"].asString());
			}*
			txt = "<b>"+threads[i]["name"].asString()+threads[i]["trip"].asString()+"</b>\n"+txt;
			row[m_post_list.text] = txt;*/
			
			ostringstream convert3;
			convert3 << threads[i]["replies"].asUInt();
			
			row[m_thread_list.text] = standardPostFormatting(threads[i])+"\n\n"+convert3.str()+" replies";
			//ostringstream convert;
			//convert << threads[i]["tim"].asString();
			//row[m_thread_list.tim] = convert.str();
			row[m_thread_list.tim] = threads[i]["tim"].asString();
			//row[m_thread_list.m_col_num] = to_string(threads[i]["tim"].asUInt64());
			row[m_thread_list.ext] = trim(threads[i]["ext"].asString());
			//row[m_thread_list.no] = trim(threads[i]["no"].asString());
			if(threads[i].isMember("filename")) {
				row[m_thread_list.fn] = threads[i]["filename"].asString();
				//row[m_thread_list.fn] = "";
			} /*else {
				row[m_thread_list.fn] = "";
			}*/
			
			ostringstream convert2;
			convert2 << threads[i]["no"].asInt();
			row[m_thread_list.no] = convert2.str();
			
			//cout << threads[i]["num"].asString() << endl;
			
			//Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
			//thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
		}
	}
	
	spin->hide();
	spin->set_visible(false);
	
	tree->show();
	tree->set_visible(true);
}

void getPostData(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	Json::Value json = getPosts(BOARD, THREAD);
	m_threads_tree_model->clear();
	//for(int j=0;j<json.size();++j) {
	Json::Value posts = json["posts"];
	for(int i=0;i<posts.size();++i) {
		Gtk::TreeModel::Row row = *(m_threads_tree_model->append());
		
		/*string txt = stripBreaks(posts[i]["com"].asString());
		
		//if(posts[i]["sub"].asString() != "") {
		if(posts[i].isMember("sub")) {
			txt = "<u>"+posts[i]["sub"].asString()+"</u>\n"+txt;
		}/ else {
			row[m_thread_list.m_col_text] = stripBreaks(threads[i]["com"].asString());
		}*
		txt = "<b>"+posts[i]["name"].asString()+posts[i]["trip"].asString()+"</b>\n"+txt;
		row[m_post_list.text] = txt;*/
		string text = standardPostFormatting(posts[i]);
		row[m_post_list.text] = text;
		//ostringstream convert;
		//convert << posts[i]["tim"].asString();
		//row[m_post_list.tim] = convert.str();
		row[m_post_list.tim] = posts[i]["tim"].asString();
		//row[m_thread_list.m_col_num] = to_string(threads[i]["tim"].asUInt64());
		row[m_post_list.ext] = trim(posts[i]["ext"].asString());
		//row[m_thread_list.no] = trim(threads[i]["no"].asString());
		
		ostringstream convert2;
		convert2 << posts[i]["no"].asInt();
		row[m_post_list.no] = convert2.str();
		
		row[m_post_list.file] = posts[i].isMember("filename") ? 1 : 0;
		
		if(posts[i].isMember("filename")) {
			row[m_post_list.fn] = posts[i]["filename"].asString();
		}
		
		/*if(posts[i].isMember("filename")) {
			row[m_post_list.thumb] = *Gdk::Pixbuf::create_from_file(getRemoteFile("https://thumbs.4chan.org"+BOARD+"thumb/"+convert.str()+"s.jpg"));
		}*/
		
		//boost::regex double_meme_arrows("\\s?&gt;&gt;.*$");
		//boost::regex double_meme_arrows("&gt;&gt;\\d*");
		boost::regex double_meme_arrows("(&gt;&gt;(?!&gt;)\\d{1,})");
		//boost::regex double_meme_arrows("\\s");
		string curPost = stripBreaks(posts[i]["com"].asString());
		boost::sregex_token_iterator iter(curPost.begin(), curPost.end(), double_meme_arrows, 0);
		boost::sregex_token_iterator end;
		
		boost::regex actual_arrows("&gt;&gt;");
		string tool = "";
		for( ; iter != end; ++iter ) {
			//cout << "\n\n\n\n\n\n\n\n\n\n#$%" << *iter << "%$#\n\n" << endl;
			//tool = tool+"<b>"+*iter+"</b>\n";
			string s = boost::regex_replace(string(*iter), actual_arrows, "");
			int s2 = atoi(s.c_str());
			for(int j=0;j<posts.size();++j) {
				if(posts[j]["no"].asInt() == s2) {
					tool = tool+"<b>"+*iter+"</b>\n"+stripTags(posts[j]["com"].asString())+"\n\n";
					break;
				}
			}
		}
		
		//row[m_post_list.tool] = "install\ngentoo";
		//if(tool != "") {
		row[m_post_list.tool] = trim(tool);
		//}
		
		//cout << threads[i]["num"].asString() << endl;
		
		//Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
		//thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
	}
	//}
}

void on_refresh_threads_clicked(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	/*Gtk::Spinner *spin = 0;
	builder->get_widget("spinner1", spin);
	spin->set_visible(true);
	spin->show();
	
	Gtk::TreeView *tree = 0;
	builder->get_widget("treeview2", tree);
	tree->hide();
	tree->set_visible(false);*/
	
	m_threads_tree_model->clear();
	getThreadData(m_threads_tree_model);
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	
	Gtk::ScrolledWindow *scroll = 0;
	builder->get_widget("scrolledwindow2", scroll);
	scroll->get_vadjustment()->set_value(0);
	
	/*spin->hide();
	spin->set_visible(false);
	
	tree->show();
	tree->set_visible(true);*/
}

void on_refresh_posts_clicked(Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
	cout << "Chimichanga!" << endl;
	Gtk::Spinner *spin = 0;
	builder->get_widget("spinner1", spin);
	spin->show();
	spin->set_visible(true);
	
	Gtk::TreeView *tree = 0;
	builder->get_widget("treeview4", tree);
	tree->hide();
	tree->set_visible(false);
	
	//sleep(3);
	
	m_threads_tree_model->clear();
	getPostData(m_threads_tree_model);
	webkit_web_view_load_uri(pWebKitView, "about:blank");
	
	spin->hide();
	spin->set_visible(false);
	
	tree->show();
	tree->set_visible(true);
}

void on_post_clicked() {
	//cout << "YOLO" << endl;
	CURL *curl;
	FILE *fp;
	CURLcode res;
	string url_string = "https://sys.4chan.org"+BOARD+"post";
	const char* url = url_string.c_str();
	//char * fn = tmpnam(NULL);
	//char outfilename[102400] = fn;
	
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
		CURLFORM_COPYNAME, "com", //the name of the data to send
		CURLFORM_COPYCONTENTS, com.c_str(), //the users username
		CURLFORM_END);

	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "recaptcha_response_field", //the name of the data to send
		CURLFORM_COPYCONTENTS, captcha_text.c_str(), //the users password
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "recaptcha_challenge_field", //the name of the data to send
		CURLFORM_COPYCONTENTS, CHALLENGE.c_str(), //the users password
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "pwd", //the name of the data to send
		CURLFORM_COPYCONTENTS, "999999", //the users password
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "MAX_FILE_SIZE", //the name of the data to send
		CURLFORM_COPYCONTENTS, "3145728", //the users password
		CURLFORM_END);
	
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "mode", //the name of the data to send
		CURLFORM_COPYCONTENTS, "regist", //the users password
		CURLFORM_END);
	
	if(THREAD != "") {
		curl_formadd(&formpost,
			&lastptr,
			CURLFORM_COPYNAME, "resto", //the name of the data to send
			CURLFORM_COPYCONTENTS, THREAD.c_str(), //the users password
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
		//fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		/*if ( (argc == 2) && (!strcmp(argv[1], "noexpectheader")) )
			// only disable 100-continue header if explicitly requested 
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);*/
		
		char * fn = tmpnam(NULL);
		fp = fopen(fn,"wb");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		
		res = curl_easy_perform(curl);
		
		//std::ifstream t(path.c_str());
		
		/* always cleanup */
		curl_easy_cleanup(curl);
		fclose(fp);
		//return string(fn);
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
				//file->set_filename("");
				file->unselect_all();
				
				Gtk::ToggleToolButton *post_but;
				builder->get_widget("toolbutton8", post_but);
				post_but->set_active(false);
				//on_refresh_posts_clicked(
				
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
		
		//refresh_captcha();
	}
	//return "";
}

bool on_key_press_or_release_event(GdkEventKey* event, Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
  if (event->type == GDK_KEY_PRESS && event->keyval == GDK_KEY_F5) {
	//cout << "yolo" << endl;
	on_refresh_threads_clicked(m_threads_tree_model);
	return true;
  }
  return false;
}
bool on_key_press_thread(GdkEventKey* event, Glib::RefPtr<Gtk::ListStore> m_threads_tree_model) {
  if (event->type == GDK_KEY_PRESS && event->keyval == GDK_KEY_F5) {
	//cout << "swag" << endl;
	on_refresh_posts_clicked(m_threads_tree_model);
	return true;
  }
  return false;
}

void open_thread_in_browser() {
	string url = "https://boards.4chan.org"+BOARD+"res/"+THREAD;
	GError *error = NULL;
	gtk_show_uri(gdk_screen_get_default(), url.c_str(), gtk_get_current_event_time(), &error);
}

string addLinks(string str) {
	boost::regex urls("((https?|ftp|file)://[-A-Z0-9+&@#/%?=~_|!:,.;]*[A-Z0-9+&@#/%=~_|])", boost::regex::icase);
	string ret = boost::regex_replace(str, urls, "<a href='$1'>$1</a>");
	return ret;
}

void on_post_activated1(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model, Glib::RefPtr<Gtk::ListStore> store) {
	Gtk::ToggleToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	
	if(!post_but->get_active()) {
		string tool = row[m_post_list.tool];
		string no = row[m_post_list.no];
		string com = row[m_post_list.text];
		/*string ext = row[m_post_list.ext];
		int f = row[m_post_list.file];
		string tim = row[m_post_list.tim];*/
		
		Gtk::Window *window = 0;
		builder->get_widget("window1", window);
	
		/*stringstream foostream(com);
		string s1;
		getline(foostream, s1);
		cout << foostream.str() << endl;*/
		boost::regex r1("^<b>.*</b> #\\d*$");
		string s1 = boost::regex_replace(com, r1, "");
		//cout << s1 << endl;
		
		Gtk::MessageDialog dialog(*window, "");
		dialog.set_secondary_text(tool+((tool != "") ? "\n\n" : "")+"<i>"+addLinks(trim(stripTags(com)))+"</i>", true);
		/*if(f) {
			string fn = getRemoteFile("https://images.4channel.org"+BOARD+"src/"+tim+ext);
			Gtk::Image img(fn);
			dialog.set_image(img);
			dialog.set_icon_from_file(fn);
		}*/
		//if(tool != "") {
		dialog.run();
		//}
	} else {
		Gtk::TextView *com_box = 0;
		builder->get_widget("textview1", com_box);
		string com = com_box->get_buffer()->get_text();
		string no = row[m_post_list.no];
		com_box->get_buffer()->set_text(com+">>"+no);
	}
}

void on_thread_activated(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model, Glib::RefPtr<Gtk::ListStore> store) {
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	string no = row[m_thread_list.no];
	//cout << "\n\n\n\n\n\n\n\n\n\n" << no << "\n\n\n\n\n\n\n\n\n\n" << endl;
	//getPostData(no);
	THREAD = no;
	
	cout << THREAD << endl;
	cout << BOARD << endl;
	
	threadView();
	//button->signal_clicked().connect(sigc::bind<Gtk::TreeModel::Path path, Gtk::TreeViewColumn*, Glib::RefPtr<Gtk::TreeModel> >(sigc::ptr_fun(&on_thread_back_clicked)), path, column, model);
	
	Gtk::TreeView *postlist = 0;
	builder->get_widget("treeview4", postlist);
	//threadview->set_visible(true);
	//threadview->show();
	postlist->set_visible(true);
	postlist->show();
	
	//if(!pWebKitView) {
	//pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	//}
	//WebKitWebSettings *settings = webkit_web_settings_new ();
	//g_object_set (G_OBJECT(settings), "auto-resize-window", TRUE, NULL);

	/* Apply the result */
	//webkit_web_view_set_settings (WEBKIT_WEB_VIEW(pWebKitView), settings);
	
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	//imgview->add(webView);
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
	
	//Gtk::CellRenderer *cellRender = 0;
	//builder->get_widget("cellrenderertext3", cellRender);
	//cellRender = threadlist->get_column_cell_renderer(0);
	//cellRender->set_property("wrap_width", 20);
	Gtk::CellRendererText* cellRender = dynamic_cast<Gtk::CellRendererText*>(postlist->get_column_cell_renderer(0));
	cellRender->property_wrap_mode() = Pango::WRAP_WORD;
	cellRender->property_wrap_width() = 400;
	//cellRender->set_padding(0, 0);
	cellRender->set_alignment(0, 0);
	
	Glib::RefPtr<Gtk::TreeSelection> thread_selection = postlist->get_selection();
	thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed_post), thread_selection, pWebKitView));
	
	getPostData(m_posts_tree_model);
	
	/*Gtk::TreeView *postview = 0;
	builder->get_widget("treeview4", postview);
	//posts->set_cursor(Gtk::TreePath("0"));*/
	/*Gtk::TreeModel::Path path = 0;
	postview->get_path_at_pos(0, 0, path);
	postview->set_cursor(path);*/
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
	//cout << "\n\n\n\n\n\n\n\n\n\n" << BOARD << "\n\n\n\n\n\n\n\n\n\n" << endl;
	
	POST_CLICK_CONNECT = postlist->signal_row_activated().connect(sigc::bind<Glib::RefPtr<Gtk::TreeModel>, Glib::RefPtr<Gtk::ListStore> >(sigc::ptr_fun(&on_post_activated1), postlist->get_model(), m_posts_tree_model));
}

void on_button_clicked(Gtk::TreeModel::Path path, Gtk::TreeViewColumn* column, Glib::RefPtr<Gtk::TreeModel> model) {
	//std::cout << "Hello World" << std::endl;
	Gtk::TreeModel::iterator iter = model->get_iter(path);
	Gtk::TreeModel::Row row = *iter;
	string shortname = row[m_columns.name];
	string longname = row[m_columns.longname];
	//cout << shortname << " - " << longname << endl;
	
	BOARD = shortname;
	LONGBOARD = longname;
	
	boardView();
	
	Gtk::TreeView *threadlist = 0;
	builder->get_widget("treeview2", threadlist);
	//threadview->set_visible(true);
	//threadview->show();
	
	//if(!pWebKitView) {
	pWebKitView = WEBKIT_WEB_VIEW (webkit_web_view_new ());
	//}
	//WebKitWebSettings *settings = webkit_web_settings_new ();
	//g_object_set (G_OBJECT(settings), "auto-resize-window", TRUE, NULL);

	/* Apply the result */
	//webkit_web_view_set_settings (WEBKIT_WEB_VIEW(pWebKitView), settings);
	
	Gtk::ScrolledWindow *imgview = 0;
	builder->get_widget("scrolledwindow3", imgview);
	imgview->set_visible(true);
	imgview->show();
	//imgview->add(webView);
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
	
	//Gtk::CellRenderer *cellRender = 0;
	//builder->get_widget("cellrenderertext3", cellRender);
	//cellRender = threadlist->get_column_cell_renderer(0);
	//cellRender->set_property("wrap_width", 20);
	Gtk::CellRendererText* cellRender = dynamic_cast<Gtk::CellRendererText*>(threadlist->get_column_cell_renderer(0));
	cellRender->property_wrap_mode() = Pango::WRAP_WORD;
	cellRender->property_wrap_width() = 400;
	//cellRender->set_padding(0, 0);
	cellRender->set_alignment(0, 0);
	
	//Json::Value json = readFile("catalog.json");
	/*Json::Value json = getThreads(BOARD);
	for(int j=0;j<json.size();++j) {
		Json::Value threads = json[j]["threads"];
		for(int i=0;i<threads.size();++i) {
			Gtk::TreeModel::Row row = *(m_threads_tree_model->append());
			
			if(threads[i]["sub"].asString() != "") {
				row[m_thread_list.m_col_text] = "<b>"+threads[i]["sub"].asString()+"</b>\n"+stripBreaks(threads[i]["com"].asString());
			} else {
				row[m_thread_list.m_col_text] = stripBreaks(threads[i]["com"].asString());
			}
			row[m_thread_list.m_col_num] = to_string(threads[i]["tim"].asUInt64());
			row[m_thread_list.m_col_ext] = threads[i]["ext"].asString();
			//cout << threads[i]["num"].asString() << endl;
			*/
		Glib::RefPtr<Gtk::TreeSelection> thread_selection = threadlist->get_selection();
		thread_selection->signal_changed().connect(sigc::bind<Glib::RefPtr<Gtk::TreeSelection>, WebKitWebView*>(sigc::ptr_fun(&on_selection_changed), thread_selection, pWebKitView));
		/*}
	}*/
	getThreadData(m_threads_tree_model);
	//cout << cellRender->get_padding() << endl;
	
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
void on_action_something(Glib::VariantBase parameter) {
	cout << "Hello, world" << endl;
}

void nameSettingsDialog() {
	//Gtk::Dialog *dialog = 0;
	//builder->get_widget("dialog1", dialog);
	Gtk::Window *window = 0;
	builder->get_widget("window1", window);
	
	Gtk::Dialog dialog("Settings", *window, true);
	
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	
	Gtk::Box *box = dialog.get_content_area();
	
	Gtk::Table box2(3, 3, true);
	
	Gtk::Label title_label;
	title_label.set_markup("<b>Posting Settings</b>\n");
	
	//Gtk::HBox part1;
	Gtk::Label name_label;
	name_label.set_text("Username ");
	Gtk::Entry name_entry;
	
	box2.attach(name_label, 0, 1, 0, 1);
	box2.attach(name_entry, 1, 3, 0, 1);
	
	//part1.pack_start(name_label);
	//part1.pack_start(name_entry);
	
	//Gtk::HBox part2;
	Gtk::Label email_label;
	email_label.set_text("Email");
	Gtk::Entry email_entry;
	
	box2.attach(email_label, 0, 1, 1, 2);
	box2.attach(email_entry, 1, 3, 1, 2);
	
	//part2.pack_start(email_label);
	//part2.pack_start(email_entry);
	
	//Gtk::HBox part3;
	Gtk::Label pwd_label;
	pwd_label.set_text("Password ");
	Gtk::Entry pwd_entry;
	pwd_entry.set_visibility(false);
	
	box2.attach(pwd_label, 0, 1, 2, 3);
	box2.attach(pwd_entry, 1, 3, 2, 3);
	
	//part3.pack_start(pwd_label, true, false);
	//part3.pack_start(pwd_entry, true, true);
	
	//box->pack_start(title_label);
	//box2.attach(part1, 0, 1, 0, 1);
	//box2.pack_start(part2);
	//box2.pack_start(part3);
	
	Gtk::Table box3(3, 3, true);
	Gtk::Label hide_label;
	hide_label.set_text("Hide Type");
	hide_label.set_tooltip_text("What to display instead when you hide images");
	Gtk::ComboBoxText hide_type;
	hide_type.append("Special Page");
	hide_type.append("Blank Page");
	hide_type.append("Cats");
	hide_type.append("Dogs");
	hide_type.set_active(0);
	
	box3.attach(hide_label, 0, 1, 0, 1);
	box3.attach(hide_type, 1, 3, 0, 1);
	
	Gtk::Notebook nb;
	nb.append_page(box2, "Posting");
	nb.append_page(box3, "Hiding");
	box->pack_start(nb);
	
	dialog.set_attached_to(*window);
	dialog.set_transient_for(*window);
	dialog.show_all();
	int resp = dialog.run();
	switch(resp) {
		case(Gtk::RESPONSE_OK): {
			cout << "OKAY!" << endl;
			break;
		} case(Gtk::RESPONSE_CANCEL): {
			cout << "CANCEL!" << endl;
			break;
		} default: {
			cout << "U WOT M8?" << endl;
			break;
		}
	}
}

void saveImageAs() {
	///Download the image of the currently selected post, with a file save dialog and everything
	
	Gtk::TreeView *treeview = 0;
	builder->get_widget("treeview4", treeview);
	
	Glib::RefPtr<Gtk::TreeSelection> tree_selection = treeview->get_selection();
	Glib::RefPtr<Gtk::TreeModel> model = treeview->get_model();
	//std::vector<TreeModel::Path> path = tree_selection->get_selected_rows(model);
	
	//Gtk::TreeModel::iterator iter = model->get_iter(path[0]);
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
		//dialog.set_transient_for(*this);
		
		dialog.set_current_name(tim+ext);
		
		//Add response buttons the the dialog:
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

		int result = dialog.run();
		switch(result) {
			case(Gtk::RESPONSE_OK):
			{
				string fn = dialog.get_filename();
				//char* cp = strdup(fn.c_str());
				getRemoteFile(url, fn);
				//free(cp);
			}
		}
	}
}

void show_post_section() {
	Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(true);
	post_grid->show();
	
	/*Gtk::TextView *view = 0;
	builder->get_widget("textview1", view);
	GtkSpellChecker* spell = gtk_spell_checker_new ();
	gtk_spell_checker_set_language (spell, "en_US", NULL);
	gtk_spell_checker_attach (spell, GTK_TEXT_VIEW (view->gobj()));*/
}
void hide_post_section() {
	Gtk::Grid *post_grid;
	builder->get_widget("grid1", post_grid);
	post_grid->set_visible(false);
	post_grid->hide();
}

/*void on_post_clicked() {
	cout << "SWAG" << endl;
}*/

/*bool refresh_captcha() {
	string s1 = getRemoteFile("http://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc");
	//fp = fopen(fn,"rb");
	/*ifstream myReadFile;
	myReadFile.open(s1);
	char output[500];
	if (myReadFile.is_open()) {
		while (!myReadFile.eof()) {
			myReadFile >> output;
			//cout<<output;
		}
	}*
	std::ifstream t(s1.c_str());
	string s2;
	t.seekg(0, ios::end);
	s2.reserve(t.tellg());
	t.seekg(0, ios::beg);
	s2.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	
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
	
	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(s7, root);
	
	//cout << "[" << root.toStyledString() << "]" << endl;
	
	//cout << root["challenge"] << endl;
	
	//Json::Value challengePage = readFile(getRemoteFile("http://www.google.com/recaptcha/api/challenge?k=6Ldp2bsSAAAAAAJ5uyx_lx34lJeEpTLVkP5k04qc"));
	//cout << challengePage.toStyledString() << endl;
	Gtk::Image *captcha = 0;
	builder->get_widget("image7", captcha);
	//captcha->set(getRemoteFile("http://www.google.com/recaptcha/api/image?c=03AHJ_VusYp6oIw56geKjgWfqSPyu_iyx1hf871VIWDnBVExgn0mhTjfSpwuTUI3B1sRtR54klJs2F13h5N307LmczsGp7qSvhJM2DL5EoK2sREsQnrX_INP60X9YH3pO0h5D5mRUignhf_QPFVmEnH08yi_zwwrQG8HifUWrvDKpcDNm1EfcMw0Q"));
	captcha->set(getRemoteFile("http://www.google.com/recaptcha/api/image?c="+root["challenge"].asString()));
	CHALLENGE = root["challenge"].asString();
	
	return true;
}*/

void on_post_button_toggled() {
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
	GtkSettings *settings;
	//settings.set_property_value("gtk-application-prefer-dark-theme", "true");
	settings = gtk_settings_get_default ();
	g_object_set (G_OBJECT (settings), "gtk-application-prefer-dark-theme", FALSE, NULL);
	
	//Glib::RefPtr<Gio::SimpleAction> m_action = Gio::SimpleAction::create("something");
	//m_action->signal_activate().connect(sigc::ptr_fun(&on_action_something));
	//app->add_action(m_action);

	Glib::RefPtr<Gio::Menu>	menu = Gio::Menu::create();
	// append to the menu three options
	menu->append("New", "app.something");
	//menu->append("About", "app.about");
	//menu->append("Quit", "app.quit");
	// set the menu as menu of the application
	app->set_app_menu(menu);
	
	//cout << "test" << endl;
	Gtk::Main kit;
	
	builder = Gtk::Builder::create_from_file(DATADIR "/main.glade");
	Gtk::Window *window = 0;
	builder->get_widget("window1", window);
	window->set_wmclass("org.gtkmm.examples.application", name);
	window->maximize();
	//app->add_window(*window);
	
	//Gtk::ApplicationWindow app_win;
	
	/*Gtk::Dialog dialog("test dialog");
	dialog.run();*/
	
	
	Gtk::TreeView *listview = 0;
	builder->get_widget("treeview1", listview);
	
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
	m_refTreeModel = Gtk::ListStore::create(m_columns);
	listview->set_model(m_refTreeModel);
	listview->set_search_column(1);
	
	Glib::RefPtr<Gtk::TreeSelection> list_selection = listview->get_selection();
	listview->signal_row_activated().connect(sigc::bind<Glib::RefPtr<Gtk::TreeModel> >(sigc::ptr_fun(&on_button_clicked), listview->get_model()));
	
	//Gtk::TreeModel::Row row = *(m_refTreeModel->append());
	//row[m_columns.m_col_short] = "/b/";
	//row[m_columns.m_col_long] = "Retards";
	
	//Gtk::TreeModel::Row row = *(list->append());
	//row["name"] = "/a/";
	//list->set_text(0, 0, "YOLO");
	//row->set_value(0, "Hi");
	
	//return app->run(window);
	//window->show();
	
	//cout << readFile("boards.json")["boards"] << endl << readFile("boards.json")["boards"].size() << endl;
	//Json::Value json = readFile("boards.json");
	Json::Value json = readFile(getRemoteFile("https://api.4chan.org/boards.json"));
	Json::Value boards = json["boards"];
	for(int i=0;i<boards.size();++i) {
		//cout << "yolo" << endl;
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_columns.name] = "/"+boards[i]["board"].asString()+"/";
		row[m_columns.longname] = boards[i]["title"].asString();
		//Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_file("/home/colin/Documents/CPP/minichan/redditUbuntu.png");
		//row[m_columns.m_col_pixbuf] = pix;
		//cout << boards[i]["title"] << endl;
	}
	listview->set_rules_hint(true);
	//cout << listview->get_rules_hint() << endl;
	
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
	
	//refresh_captcha();
	
	//Gtk::Grid *post_grid;
	//builder->get_widget("grid1", post_grid);
	Gtk::ToolButton *post_but;
	builder->get_widget("toolbutton8", post_but);
	post_but->signal_clicked().connect(sigc::ptr_fun(&on_post_button_toggled));
	
	//Gtk::Action *hide = 0;
	//hide = builder->get_object("action1");
	//hide->signal_activate().connect(sigc::ptr_fun(&on_hide_toggled));
	
	
	Gtk::Button *post_but2 = 0;
	builder->get_widget("button4", post_but2);
	post_but2->signal_clicked().connect(sigc::ptr_fun(&on_post_clicked));
	
	//listview->set_pixbuf_column(2);
	kit.run(*window);
	//app->run(*window);
}

int main(int argc, char *argv[]) {
	//Gtk::Main kit(argc, argv);
	
	//Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.gtkmm.examples.base");
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create("org.gtkmm.examples.application");
	app->signal_startup().connect(sigc::bind< Glib::RefPtr<Gtk::Application> >(sigc::ptr_fun(&startup), app));
	//app->register_application();
	
	
	//cout << "[" << stripBreaks(string("hello world!123")) << "]" << endl;
//	cout << DATADIR << endl;
	
	//kit.run(*window);
	app->run(argc, argv);
	return 0;
}

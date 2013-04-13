#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_FORMATTING
#define MINICHAN_FORMATTING

using namespace std;

string stripTags(string s) {
	///Strip the HTML tags from a string
	
	boost::regex br("<br?>"); //Add a special case for <br>, turn it into \n
	string s2 = boost::regex_replace(s, br, "\n");
	
	boost::regex html("<(.|\n)*?>");
	string s3 = boost::regex_replace(s2, html, "");
	
	return s3;
}

string stripBreaks(string s) {
	///This is a bit of a misdinomer... It's really for adding special formatting to posts
	
	boost::regex meme_arrows("^\\s?(&gt;(?!&gt;).*?)$"); // >implying
	boost::regex double_meme_arrows("(&gt;&gt;(?!&gt;)\\d{1,})"); // >>26666666
	boost::regex triple_meme_arrows("^\\s(&gt;&gt;&gt;.*?)$"); // >>>/v/
	
	string s3 = stripTags(s);
	string s4 = boost::regex_replace(s3, meme_arrows, "<i>$1</i>");
	string s5 = boost::regex_replace(s4, double_meme_arrows, "<span foreground='purple'>$1</span>");
	string s6 = boost::regex_replace(s5, triple_meme_arrows, "<span foreground='red'>$1</span>");
	
	boost::regex urls("((https?|ftp|file)://[-A-Z0-9+&@#/%?=~_|!:,.;]*[A-Z0-9+&@#/%=~_|])", boost::regex::icase);
	string s7 = boost::regex_replace(s5, urls, "<span foreground='blue'>$1</span>");
	
	return s7;
}

string trim(string s, const char* t = " \t\n\r\f\v") {
	///Remove surrounding whitespace from a string
	
	s.erase(0, s.find_first_not_of(t));
	s.erase(s.find_last_not_of(t) + 1);
	return s;
}

string addLinks(string str) {
	///Detect links and add <a> tags around them
	
	boost::regex urls("((https?|ftp|file)://[-A-Z0-9+&@#/%?=~_|!:,.;]*[A-Z0-9+&@#/%=~_|])", boost::regex::icase);
	string ret = boost::regex_replace(str, urls, "<a href='$1'>$1</a>");
	return ret;
}

string standardPostFormatting(Json::Value post) {
	///Formatting that should be applied to every post (incl. OPs)
	
	string txt = stripBreaks(post["com"].asString());
	
	if(post.isMember("sub")) {
		txt = "<u>"+post["sub"].asString()+"</u>\n"+txt;
	}
	
	ostringstream convert;
	convert << post["no"].asInt();
	txt = "<b>"+post["name"].asString()+post["trip"].asString()+"</b> #"+convert.str()+"\n"+txt;
	return txt;
}

#endif // MINICHAN_FORMATTING

#include <gtkmm.h>
#include <string.h>

#ifndef MINICHAN_JSON_FUNCS
#define MINICHAN_JSON_FUNCS

Json::Value readFile(string path, bool timFix=false) {
	///Read a file and parse it as JSON
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

#endif // MINICHAN_JSON_FUNCS

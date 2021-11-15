#pragma once
#include <string>

#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Normaliz.lib")

using namespace std;

class mycurl
{
public:
	string get(string url);
	string escape(string s);
};


#ifndef __STRING_UTILS__H__
#define __STRING_UTILS__H__

#include<string>
#include<vector>
#include<algorithm>

using std::string;
using std::vector;
using std::string_view;

class StringUtils {
public:
    static bool isEmpty(const string &str);
    static string trim(const string & str);
    static string joinString(const vector<string> & strVec, string_view delimiter);
    static vector<string> split(const string & str, string_view delimiter);
    static string toLower(const string & str);
    static string toUpper(const string & str);
    static unsigned char toLower(const unsigned char ch);
    static unsigned char toUpper(const unsigned char ch);
    static bool isStartWith(string_view str, string_view pattern);
    static bool isEndWith(string_view str, string_view pattern);
    static bool equalIgnoreCases(string_view str1, string_view str2);
};

#endif
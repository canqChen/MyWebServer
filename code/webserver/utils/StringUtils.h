#ifndef __STRING_UTILS__H__
#define __STRING_UTILS__H__

#include<string>
#include<vector>
#include<algorithm>

using std::string;
using std::vector;

class StringUtils {
public:
    static bool isEmpty(const string &str);
    static string trim(const string & str);
    static string joinString(const vector<string> & strVec, string delimiter="");
    static vector<string> split(const string & str, string delimiter);
    static string toLower(const string & str);
    static string toUpper(const string & str);
    static unsigned char toLower(unsigned char ch);
    static unsigned char toUpper(unsigned char ch);
    static bool startWith(const string & str, const string & pattern);
    static bool endWith(const string & str, const string & pattern);
};

#endif
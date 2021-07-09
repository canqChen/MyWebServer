#ifndef __STRING_UTILS__H__
#define __STRING_UTILS__H__

# include<string>
# include<vector>

using std::string;
using std::vector;

class StringUtils {
public:
    static bool isEmpty(const string &str);
    static string trim(const string & str);
    static string joinString(const vector<string> & strVec, string delimiter="");
};

#endif
# include "StringUtils.h"

bool StringUtils::isEmpty(const string &str) {
    if(str.empty())
        return true;
    string whiteSpaces(" \t\f\v\n\r");
    auto startIdx = str.find_first_not_of(whiteSpaces);
    if(startIdx == str.npos) {
        return true;
    }
    return false;
}

// 去除字符串两边空格
string StringUtils::trim(const string & str) {
    string whiteSpaces(" \t\f\v\n\r");
    auto startIdx = str.find_first_not_of(whiteSpaces);
    auto endIdx = str.find_last_not_of(whiteSpaces);
    if(startIdx == str.npos) {
        return "";
    }
    return str.substr(startIdx, endIdx - startIdx + 1);
}

// 拼接字符串，以delimiter为分界
string StringUtils::joinString(const vector<string> & strVec, string delimiter="") {
    string ret;
    int len = strVec.size();
    for(int i = 0; i < len; ++i) {
        ret += strVec[i];
        if(i != len - 1)
            ret += delimiter;
    }
    return ret;
}

vector<string> StringUtils::split(const string & str, string delimiter) {
    if(delimiter == "")
        return {str};
    vector<string> ret;
    auto it = str.begin();
    auto start = str.begin();
    while(it != str.end()) {
        it = std::search(start, str.end(), delimiter.begin(), delimiter.end());
        if(it > start){
            ret.emplace_back(str.substr(start - str.begin(), it - start));
        }
        start = it + delimiter.size();
    }
    return ret;
}

unsigned char StringUtils::toLower(unsigned char ch) {
    if(ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 'a';
    }
    return ch;
}

unsigned char StringUtils::toUpper(unsigned char ch) {
    if(ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 'A';
    }
    return ch;
}

string StringUtils::toLower(const string & str) {
    string ret;
    for(auto &ch : str) {
        ret.push_back(toLower(ch));
    }
    return ret;
}

string StringUtils::toUpper(const string & str) {
    string ret;
    for(auto &ch : str) {
        ret.push_back(toUpper(ch));
    }
    return ret;
}
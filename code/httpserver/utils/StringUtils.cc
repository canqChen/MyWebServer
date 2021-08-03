#include<regex> 
#include "httpserver/utils/StringUtils.h"


bool StringUtils::isEmpty(string_view str) 
{
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
string StringUtils::trim(string_view str) 
{
    string whiteSpaces(" \t\f\v\n\r");
    auto startIdx = str.find_first_not_of(whiteSpaces);
    auto endIdx = str.find_last_not_of(whiteSpaces);
    if(startIdx == str.npos) {
        return "";
    }
    return str.substr(startIdx, endIdx - startIdx + 1);
}

// 拼接字符串，以delimiter为分界
string StringUtils::joinString(const vector<string> & strVec, string_view delimiter) 
{
    string ret;
    int len = strVec.size();
    for(int i = 0; i < len; ++i) {
        ret += strVec[i];
        if(i != len - 1)
            ret += delimiter;
    }
    return ret;
}

vector<string> StringUtils::split(string_view str, string_view delimiter) 
{
    if(delimiter == "")
        return {str.data()};
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

unsigned char StringUtils::toLower(const unsigned char ch) 
{
    if(ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 'a';
    }
    return ch;
}

unsigned char StringUtils::toUpper(const unsigned char ch) 
{
    if(ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 'A';
    }
    return ch;
}

string StringUtils::toLower(string_view str) 
{
    string ret;
    for(auto &ch : str) {
        ret.push_back(toLower(ch));
    }
    return ret;
}

string StringUtils::toUpper(string_view str) 
{
    string ret;
    for(auto &ch : str) {
        ret.push_back(toUpper(ch));
    }
    return ret;
}

bool StringUtils::isStartWith(string_view str, string_view pattern) 
{
    string s(str);
    string ps = "^" + string(pattern) + ".*";
    std::regex p(ps);
    std::smatch sm;
    if(std::regex_match(s, sm, p)) {
        return true;
    }
    return false;
}

bool StringUtils::isEndWith(string_view str, string_view pattern) 
{
    string s(str);
    string ps = ".*" + string(pattern) + "$";
    std::regex p(ps);
    std::smatch sm;
    if(std::regex_match(s, sm, p)) {
        return true;
    }
    return false;
}

bool StringUtils::equalIgnoreCases(string_view str1, string_view str2) 
{
    if(str1.size() != str2.size()) {
        return false;
    }
    for(int i = 0; i < str1.size(); ++i) {
        if(toLower(str1[i]) != toLower(str2[i])) {
            return false;
        }
    }
    return true;
}

string StringUtils::getSuffix(string_view filename) 
{
    string tmp(filename);
    auto idx = tmp.find_last_of('.');
    if(idx == tmp.npos) {
        return "";
    }
    return tmp.substr(idx);
}
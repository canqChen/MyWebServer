#ifndef CODE_CONVERTOR
#define CODE_CONVERTOR

#include<string>
#include<memory>
#include<codecvt>
#include<iostream>
#include<locale>

using std::string;
using std::wstring;

class CodeConvertor {
    static string UnicodeToUTF8(const wstring & wstr);
    static wstring UTF8ToUnicode(const string & str);
    static string UnicodeToANSI(const wstring & wstr);
    static wstring ANSIToUnicode(const string & str);
    static string UTF8ToANSI(const string & str);
    static string ANSIToUTF8(const string & str);
};


#endif
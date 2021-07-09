#ifndef URLENCODEUTILS_H
#define URLENCODEUTILS_H

# include<string>
# include<stdexcept>
# include<assert.h>

using std::string;

class UrlEncodeUtils {
    static unsigned char toHex(unsigned char ch);
    static unsigned char fromHex(unsigned char ch);
    static string decode(const string & str);
    static string encode(const string & str);
};


#endif
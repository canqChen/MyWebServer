#ifndef URLENCODEUTILS_H
#define URLENCODEUTILS_H

# include<string>
# include<stdexcept>
# include<assert.h>

using std::string;
typedef unsigned char byte;

class URLEncodeUtils {
    static byte toHex(byte ch);
    static byte fromHex(byte ch);
    static string decode(const string & str);
    static string encode(const string & str);
};


#endif
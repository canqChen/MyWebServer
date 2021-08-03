#ifndef URLENCODEUTILS_H
#define URLENCODEUTILS_H

# include <string>
# include <stdexcept>
# include <assert.h>

using std::string;
using std::string_view;
typedef unsigned char byte;

class URLEncodeUtils {
    static byte toHex(byte ch);
    static byte fromHex(byte ch);
    static string decode(string_view str, bool URI = true);
    static string encode(string_view str, bool URI = true);
};


#endif
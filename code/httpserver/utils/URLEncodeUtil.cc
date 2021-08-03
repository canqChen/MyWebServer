# include "URLEncodeUtil.h"


byte URLEncodeUtils::toHex(byte x) {
    return  x > 9 ? x + 55 : x + 48; 
}

byte URLEncodeUtils::fromHex(byte x) {
    byte y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    return y;
}

string URLEncodeUtils::decode(string_view str, bool URI = true) {
    string ret;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++) {
        if (str[i] == '+' && !URI)  // application/x-www-form-urlencoded 格式里，空格 encode 为 +
            ret += ' ';
        else if (str[i] == '%') {
            assert(i + 2 < length);
            byte high = fromHex((byte)str[++i]);
            byte low = fromHex((byte)str[++i]);
            ret += high * 16 + low;
        }
        else 
            ret += str[i];
    }
    return ret;
}


// 编码：用于键值对参数，参数之间用&间隔, 如果有空格，将空格转换为+加号；=号前是key；有特殊符号，用%标记，并将特殊符号转换为ASCII HEX值
/*
1. 字母数字字符 "a" 到 "z"、"A" 到 "Z" 和 "0" 到 "9" 保持不变。

2. 特殊字符 "."、"-"、"*" 和 "_" 保持不变。

3. 空格字符 " " 转换为一个加号 "+"

4. 所有其他字符都是不安全的，因此首先使用一些编码机制将它们转换为一个或多个字节。然后每个字节用一个包含 3 个字符的字符串 "%xy" 表示，
其中 xy 为该字节的两位十六进制表示形式。编码机制是 UTF-8。
*/

string URLEncodeUtils::encode(string_view str, bool URI = true) {
    string ret;
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum(str[i]) || (str[i] == '-') 
            || (str[i] == '_') || (str[i] == '.') 
            || (str[i] == '~')) {
            ret += str[i];
        }
        else if (str[i] == ' ' && !URI) {   // application/x-www-form-urlencoded 格式里，空格 encode 为 +
            ret += "+";
        }
        else {
            ret += '%';
            ret += toHex((byte)str[i] >> 4);
            ret += toHex((byte)str[i] % 16);
        }
    }
    return ret;
}
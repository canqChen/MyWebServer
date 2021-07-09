#ifndef MD5_H
#define MD5_H
 
#include <string>
#include <fstream>
 
/* Type define */
typedef unsigned char byte;
typedef unsigned long ulong;
 
using std::string;
using std::ifstream;
 
/* MD5 declaration. */
class MD5 {
public:
    MD5();
    MD5(const void *input, size_t length);
    MD5(const string &str);
    MD5(ifstream &in);
    void update(const void *input, size_t length);
    void update(const string &str);
    void update(ifstream &in);
    const byte* digest();
    string toString();
    void reset();

    /* class uncopyable */
    MD5(const MD5&) = delete;
    MD5(MD5&&) = delete;
    MD5& operator=(const MD5&) = delete;
private:
    void __update(const byte *input, size_t length);
    void __final();
    void __transform(const byte block[64]);
    void __encode(const ulong *input, byte *output, size_t length);
    void __decode(const byte *input, ulong *output, size_t length);
    string bytesToHexString(const byte *input, size_t length);
 
private:
    ulong state_[4];    /* state (ABCD) */
    ulong count_[2];    /* number of bits, modulo 2^64 (low-order word first) */
    byte buffer_[64];    /* input buffer */
    byte digest_[16];    /* message digest */
    bool finished_;     /* calculate finished ? */
 
    static const byte PADDING[64];    /* padding for calculate */
    static const char HEX[16];
    static const size_t BUFFER_SIZE = 1024;
};
 
#endif/*MD5_H*/
#ifndef __COOKIE__H__
#define __COOKIE__H__

# include<string>
# include "../utils/StringUtils.h"

using std::string;

class Cookie {
public:
    explicit Cookie(string name, string value, string domain="/", string path="", 
    long maxAge=-1, bool httpOnly=false, bool secure=false);
    ~Cookie(){};
    void setDomain(string domain);
    void setPath(string path);
    void setMaxAge(long maxAge);
    void setHttpOnly(bool httpOnly);
    void setSecure(bool secure);
    string getCookieStr() const;
private:
    string name_;
    string value_;
    string domain_;
    string path_;
    long maxAge_; // s, < 0 会话有效，0 删除，> 0 有效期为maxAge秒
    bool httpOnly_;
    bool secure_; 
};

#endif
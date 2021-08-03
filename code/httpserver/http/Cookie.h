#ifndef __COOKIE__H__
#define __COOKIE__H__

# include<string>
# include "../utils/StringUtils.h"

using std::string;
using std::string_view;

class Cookie {
public:
    explicit Cookie(string_view name="", string_view value="", string_view domain="/", string_view path="", 
    long maxAge=-1, bool httpOnly=false, bool secure=false);
    ~Cookie(){};
    void setName(string_view name) 
    {
        name_ = name;
    }

    void setValue(string_view value) 
    {
        value_ = value;
    }

    void setDomain(string_view domain) 
    {
        this->domain_ = domain;
    }

    void setPath(string_view path) 
    {
        this->path_ = path;
    }

    void setMaxAge(long maxAge) 
    {
        this->maxAge_ = maxAge;
    }

    void setHttpOnly(bool httpOnly) 
    {
        this->httpOnly_ = httpOnly;
    }

    void setSecure(bool secure) 
    {
        this->secure_ = secure;
    }

    string getName() const 
    {
        return name_;
    }

    string getValue() const 
    {
        return value_;
    }

    bool isEmpty()
    {
        return StringUtils::isEmpty(name_) 
            || StringUtils::isEmpty(value_);
    }

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
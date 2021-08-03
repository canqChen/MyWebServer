# include "httpserver/http/Cookie.h"

Cookie::Cookie(string_view name, string_view value, string_view domain, string_view path, 
    long maxAge, bool httpOnly, bool secure): name_(name), value_(value),
    domain_(domain), path_(path), maxAge_(maxAge), httpOnly_(httpOnly), secure_(secure)
    {}

string Cookie::getCookieStr() const {
    string ret;
    ret += name_ + ":" + value_;
    ret += " ;max-age:" + std::to_string(maxAge_);
    if(!StringUtils::isEmpty(domain_)) {
        ret += "domain=" + domain_;
    }
    if(!StringUtils::isEmpty(path_)) {
        ret += " ;path=" + path_;
    }
    if(secure_) {
        ret += " ;secure";
    }
    if(httpOnly_) {
        ret += " ;HttpOnly";
    }
    return ret;
}
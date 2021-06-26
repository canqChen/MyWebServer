# include"Cookie.h"

Cookie::Cookie(string name, string value, string domain="", string path="/", 
    long maxAge=-1, bool httpOnly=false, bool secure=false): name_(name), value_(value),
    domain_(domain), path_(path), maxAge_(maxAge), httpOnly_(httpOnly), secure_(secure)
    {}

void Cookie::setDomain(string domain) {
    this->domain_ = domain;
}

void Cookie::setPath(string path) {
    this->path_ = path;
}

void Cookie::setMaxAge(long maxAge) {
    this->maxAge_ = maxAge;
}

void Cookie::setHttpOnly(bool httpOnly) {
    this->httpOnly_ = httpOnly;
}

void Cookie::setSecure(bool secure) {
    this->secure_ = secure;
}

string Cookie::getCookieStr() const {
    string ret;
    ret += name_ + ":" + value_ + "; ";
    if(!StringUtils::isEmpty(domain_)) {
        ret += "domain=" + domain_;
    }
    if(!StringUtils::isEmpty(path_)) {
        ret += " ;path=" + path_;
    }
    ret += " ;max-age:" + std::to_string(maxAge_);
    if(secure_) {
        ret += " ;secure";
    }
    if(httpOnly_) {
        ret += " ;HttpOnly";
    }
}
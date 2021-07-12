#include<cassert>
#include "./HttpRequest.h"


HttpRequest::HttpRequest() : URL_(""), URI_(""), requestBody_(""), 
    requestMethod_(HttpMethod::UNKNOWN), statusCode_(HttpStatusCode::OK200), 
    httpVersion_(HttpVersion::ErrorVersion)
{
}

bool HttpRequest::isKeepAlive() const {
    if(requestHeaders_.count(HeaderString::CONNECTION) == 1) {
        return requestHeaders_.at(HeaderString::CONNECTION) == "keep-alive"
        && static_cast<int>(httpVersion_) >= static_cast<int>(HttpVersion::Version1_1);
    }
    return false;
}

string HttpRequest::getHeader(const string & header) const {
    string tmpHeader = StringUtils::toLower(header);
    if(requestHeaders_.count(tmpHeader)) {
        return requestHeaders_.at(tmpHeader);
    }
    return "";
}

string HttpRequest::getParameter(const string & name) const {
    assert(!name.empty());
    if(requestParameters_.count(name) == 1) {
        return requestParameters_.at(name);
    }
    return "";
}

string HttpRequest::getParameter(const char* name) const {
    assert(name != nullptr);
    string key(name);
    return getParameter(key);
}

string HttpRequest::getContentType() const {
    if(requestHeaders_.count(HeaderString::CONTENT_TYPE)) {
        return requestHeaders_.at(HeaderString::CONTENT_TYPE);
    }
    return "";
}

string HttpRequest::getCookie(const string &cookieName) {
    assert(!cookieName.empty())
    if(cookies_.count(cookieName) > 0) {
        return cookies_[cookieName];
    }
    return "";
}

string HttpRequest::getCookie(const char *cookieName) {
    assert(cookieName != nullptr);
    string name(cookieName);
    return getCookie(name);
}


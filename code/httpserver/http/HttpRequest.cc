#include<cassert>
#include "./HttpRequest.h"
#include "../utils/StringUtils.h"


HttpRequest::HttpRequest() : URL_(""), URI_(""), requestBody_(""), 
    requestMethod_(HttpMethod::UNKNOWN), httpVersion_(HttpVersion::ErrorVersion)
{
}

bool HttpRequest::isKeepAlive() const {
    string head = StringUtils::toLower(HttpHeaderName::CONNECTION);
    if(requestHeaders_.find(head) != requestHeaders_.end()) {
        return StringUtils::equalIgnoreCases(requestHeaders_.at(head), "keep-alive");
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

size_t HttpRequest::getContentLength() const {
    if(getRequestMethod() == HttpMethod::GET) {
        return 0;
    }
    int len = stoi(getParameter(HttpHeaderName::CONTENT_LENGTH));
    return static_cast<size_t>(len);
}

string HttpRequest::getParameter(const string & name) const {
    assert(!name.empty());
    string head = StringUtils::toLower(name);
    if(requestParameters_.find(head) != requestHeaders_.end()) {
        return requestParameters_.at(head);
    }
    return "";
}

string HttpRequest::getParameter(const char* name) const {
    assert(name != nullptr);
    string key(name);
    return getParameter(key);
}

string HttpRequest::getContentType() const {
    string head = StringUtils::toLower(HttpHeaderName::CONTENT_TYPE);
    if(requestHeaders_.find(head) != requestHeaders_.end()) {
        return requestHeaders_.at(head);
    }
    return "";
}

string HttpRequest::getCookie(const string &cookieName) {
    assert(!cookieName.empty());
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


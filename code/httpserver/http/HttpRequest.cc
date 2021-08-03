#include "httpserver/http/HttpRequest.h"
#include<cassert>

#include "httpserver/utils/StringUtils.h"


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

string HttpRequest::getHeader(string_view header) const {
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

string HttpRequest::getParameter(string_view name) const {
    assert(!name.empty());
    string head = StringUtils::toLower(name);
    if(requestParameters_.find(head) != requestHeaders_.end()) {
        return requestParameters_.at(head);
    }
    return "";
}


string HttpRequest::getContentType() const {
    string head = StringUtils::toLower(HttpHeaderName::CONTENT_TYPE);
    if(requestHeaders_.find(head) != requestHeaders_.end()) {
        return requestHeaders_.at(head);
    }
    return "";
}

Cookie HttpRequest::getCookieByName(string_view cookieName) {
    assert(!cookieName.empty());
    string name = string(cookieName);
    if(cookies_.count(name) > 0) {
        return cookies_[name];
    }
    return Cookie();
}


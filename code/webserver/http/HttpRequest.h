
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <string>

#include "./HttpUtils.h"

using std::string;
class HttpParser;

class HttpRequest {
public:
    friend class HttpParser;
    HttpRequest();

    ~HttpRequest() = default;
    
    string getRequestURI() const {
        return URI_;
    }

    HttpVersion getHttpVersion() const {
        return httpVersion_;
    }

    HttpMethod getMethod() const {
        return requestMethod_;
    }
    
    HttpVersion getHttpVersion() const {
        return httpVersion_;
    }

    HttpStatusCode getStatusCode() const{
        return statusCode_;
    }

    bool isKeepAlive() const;
    string getHeader(const string & header) const;
    string getParameter(const string & name) const;
    string getParameter(const char* name) const;
    string getContentType() const;
    string getCookie(const string &cookieName);
    string getCookie(const char *cookieName);

private:
    HttpMethod requestMethod_;
    string URL_, URI_, requestBody_;
    HttpVersion httpVersion_;
    std::unordered_map<string, string> requestHeaders_;
    std::unordered_map<string, string> requestParameters_;
    std::unordered_map<string, string> cookies_;
    HttpStatusCode statusCode_;
};


#endif //HTTP_REQUEST_H
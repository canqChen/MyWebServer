
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

    string getHttpVersion() const {
        return httpVersion_;
    }

    string getRequestMethod() const {
        return requestMethod_;
    }
    
    string getHttpVersion() const {
        return httpVersion_;
    }

    // for other conversion
    string getRequestBody() const{
        return requestBody_;
    }

    bool isKeepAlive() const;
    string getHeader(const string & header) const;
    string getParameter(const string & name) const;
    string getParameter(const char* name) const;
    string getContentType() const;
    size_t getContentLength() const;
    string getCookie(const string &cookieName);
    string getCookie(const char *cookieName);

private:
    string requestMethod_;
    string URL_, URI_, requestBody_;
    string httpVersion_;
    std::unordered_map<string, string> requestHeaders_;
    std::unordered_map<string, string> requestParameters_;
    std::unordered_map<string, string> cookies_;
};


#endif //HTTP_REQUEST_H
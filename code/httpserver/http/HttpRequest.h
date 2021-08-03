
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <unordered_map>
#include <string>

#include "./HttpUtils.h"
#include "./Cookie.h"

using std::string;
using std::string_view;
class HttpCodec;

class HttpRequest 
{
public:
    friend class HttpCodec;
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
    string getHeader(string_view header) const;
    string getParameter(string_view name) const;
    string getContentType() const;
    size_t getContentLength() const;
    Cookie getCookieByName(string_view cookieName);
private:
    string requestMethod_;
    string URL_, URI_, requestBody_;
    string httpVersion_;
    std::unordered_map<string, string> requestHeaders_;
    std::unordered_map<string, string> requestParameters_;
    std::unordered_map<string, Cookie> cookies_;
};


#endif //HTTP_REQUEST_H
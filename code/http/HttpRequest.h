
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/Buffer.h"
#include "../log/Log.h"
#include "../pool/SqlConnPool.h"
#include "../pool/SqlConnRAII.h"
#include "HttpUtils.h"
#include "../utils/URLEncodeUtil.h"

using std::string;

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };
    
    explicit HttpRequest(Buffer & buff) { __init(buffs); }
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

    bool isKeepAlive() const {
        if(requestHeaders_.count("Connection")) {
            return requestHeaders_.at("Connection") == "keep-alive";
        }
        return false;
    }

    string getHeader(const string & header) const {
        if(requestHeaders_.count(header)) {
            return requestHeaders_.at(header);
        }
        return "";
    }

    string getParameter(const string & name) const {
        assert(key != "");
        if(requestParameters_.count(name) == 1) {
            return requestParameters_.at(name);
        }
        return "";
    }

    string getParameter(const char* name) const {
        if(name == nullptr)
            return "";
        string key(name);
        return getParameter(key);
    }

    string getContentType() const {
        string header = "Content-Type";
        if(requestHeaders_.count(header)) {
            return requestHeaders_.at(header);
        }
        return "";
    }

    /* 
    TODO:
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    void __init(Buffer & buff);
    void __parseRequest(Buffer& buff);
    bool __parseRequestLine(const string& line);
    bool __parseHeader(const string& line);
    bool __parseBody(const string& line);
    void __parseURL(string & url);
    void __parsePostBody();
    void __parseParams(string & params);

    static bool UserVerify(const string& name, const string& pwd, bool isLogin);

private:
    PARSE_STATE parseState_;
    HttpMethod requestMethod_;
    string URL_, requestBody_, URI_;
    HttpVersion httpVersion_;
    std::unordered_map<string, string> requestHeaders_;
    std::unordered_map<string, string> requestParameters_;
    HttpStatusCode statusCode_;
};


#endif //HTTP_REQUEST_H
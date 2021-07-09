
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>
#include <errno.h>     
#include <mysql/mysql.h>  //mysql

#include "../buffer/Buffer.h"
#include "../log/Log.h"
#include "../pool/SqlConnPool.h"
#include "../pool/SqlConnRAII.h"
#include "HttpUtils.h"

using std::string;
using std::unordered_set;
using std::unordered_map;

class HttpRequest {
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };
    
    HttpRequest() { init(); }
    ~HttpRequest() = default;

    void init();
    void parseRequest(Buffer& buff);

    string getRequestURI() const;
    HttpMethod getMethod() const;
    string getHttpVersion() const;
    string getParameter(const string& name) const;
    string getParameter(const char* name) const;

    bool isKeepAlive() const;
    void getHeader() const;

    /* 
    TODO:
    void HttpConn::ParseFormData() {}
    void HttpConn::ParseJson() {}
    */

private:
    bool __parseRequestLine(const string& line);
    bool __parseHeader(const string& line);
    bool __parseBody(const string& line);
    void __parseGetParameters();
    void __parsePostBody();
    void __parseFromUrlencoded();

    static bool UserVerify(const string& name, const string& pwd, bool isLogin);

    PARSE_STATE parseState_;
    HttpMethod requestMethod_;
    string requestURI_, requestBody_, httpVersion_, requestURL_;
    map<string, string> requestHeaders_;
    map<string, string> requestParameters_;
    const char * CRLF = "\r\n"; 
    HttpStatusCode statusCode_;
    const unordered_map<string, HttpMethod> supportedMethodMap_ = {
        {"GET", GET}, {"POST", POST}, {"PUT", PUT}, {"DELETE", DELETE}
    };
};


#endif //HTTP_REQUEST_H
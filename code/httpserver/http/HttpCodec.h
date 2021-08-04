#ifndef HTTPCODEC_H
#define HTTPCODEC_H

#include "HttpResponse.h"
#include "HttpRequest.h"
#include "../HttpCallbacks.h"

class HttpCodec: NoCopyable
{
public:
    HttpCodec();
    ~HttpCodec() = default;

    HttpRequestPtr parseHttp(Buffer & buff);
    void wrapHttp(const HttpResponsePtr& resp, Buffer & buff);
private:
    // parse http
    bool __parseRequest(Buffer& buff, HttpRequestPtr& req);
    bool __parseRequestLine(const string& line, HttpRequestPtr& req);
    bool __parseHeader(const string& line, HttpRequestPtr& req);
    bool __parseBody(const string& line, HttpRequestPtr& req);
    void __parseURL(string & url, HttpRequestPtr& req);
    void __parsePostBody(const string & body, HttpRequestPtr& req);
    void __parseParams(string & params, HttpRequestPtr& req);
    void __parseCookie(string & cookies, HttpRequestPtr& req);

    /* 
    TODO:
    void parseFormData() {}
    void parseJson() {}
    */

    // wrapHttp
    void __setStatusLine(const HttpResponsePtr& resp, Buffer & buff);
    void __setHeaders(const HttpResponsePtr& resp, Buffer & buff);
    void __setBody(const HttpResponsePtr& resp, Buffer & buff);
    void __setErrorBody(const HttpResponsePtr& resp, Buffer& buff, string_view message);
private:
    enum ParseState {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };
    ParseState parseState_;
    std::string contextPath_;
    static std::unordered_map<string, std::pair<char *, struct stat> > cache_; // munmap(ptr, size);
};

#endif
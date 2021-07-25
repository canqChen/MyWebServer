#ifndef HTTPCODEC_H
#define HTTPCODEC_H

#include "./HttpResponse.h"
#include "./HttpRequest.h"
#include "../HttpCallbacks.h"

class HttpCodec: NoCopyable
{
public:
    HttpCodec(): parseState_(REQUEST_LINE);
    std::unique_ptr<HttpRequest> parseHttp(Buffer & buff);
    string wrapHttp(const std::unique_ptr<HttpResponse> & resp);
    ~HttpCodec() = default;
private:
    // parse http
    bool __parseRequest(Buffer& buff, std::unique_ptr<HttpRequest>& req);
    bool __parseRequestLine(const string& line, std::unique_ptr<HttpRequest>& req);
    bool __parseHeader(const string& line, std::unique_ptr<HttpRequest>& req);
    bool __parseBody(const string& line, std::unique_ptr<HttpRequest>& req);
    void __parseURL(string & url, std::unique_ptr<HttpRequest>& req);
    void __parsePostBody(const string & body, std::unique_ptr<HttpRequest>& req);
    void __parseParams(string & params, std::unique_ptr<HttpRequest>& req);
    void __parseCookie(string & cookies, std::unique_ptr<HttpRequest>& req);

    /* 
    TODO:
    void parseFormData() {}
    void parseJson() {}
    */

private:
    enum ParseState {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };
    ParseState parseState_;

};

#endif
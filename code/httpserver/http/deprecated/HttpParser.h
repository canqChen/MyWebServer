#if 0

#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <string>
#include <memory>

#include "../../common/NoCopyable.h"

using std::string;
class Buffer;
class HttpRequest;

class HttpParser : NoCopyable {
public:
    static bool parse(Buffer & buff, std::unique_ptr<HttpRequest>& req) {
        PARSE_STATE = REQUEST_LINE;
        __parseRequest(buff, req);
    }
private:
    static PARSE_STATE parseState_;
    static bool __parseRequest(Buffer& buff, std::unique_ptr<HttpRequest>& req);
    static bool __parseRequestLine(const string& line, std::unique_ptr<HttpRequest>& req);
    static bool __parseHeader(const string& line, std::unique_ptr<HttpRequest>& req);
    static bool __parseBody(const string& line, std::unique_ptr<HttpRequest>& req);
    static void __parseURL(string & url, std::unique_ptr<HttpRequest>& req);
    static void __parsePostBody(const string & body, std::unique_ptr<HttpRequest>& req);
    static void __parseParams(string & params, std::unique_ptr<HttpRequest>& req);
    static void __parseCookie(string & cookies, std::unique_ptr<HttpRequest>& req);

    /* 
    TODO:
    void ParseFormData() {}
    void ParseJson() {}
    */

private:
    static enum PARSE_STATE {
        REQUEST_LINE,
        HEADER,
        BODY,
        FINISH,
    };
};

#endif

#endif
#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <unordered_map>
#include <unordered_set>
#include <string>

#include "../Config.h"

using std::string;
using std::string_view;
using std::unordered_map;
using std::unordered_set;

struct HttpMethod 
{
    static const string UNKNOWN = "unknown";
    static const string HEAD = "HEAD";
    static const string GET = "GET";
    static const string POST = "POST";
    static const string PUT = "PUT";
    static const string DELETE = "DELETE";

    static const unordered_set<string> methodTable_ {
        GET, POST
    };
    
    static bool contain(string_view method) 
    {
        string tmp(method);
        if(methodTable_.find(tmp) != methodTable_.end()) {
            return true;
        }
        return false;
    }
};

struct HttpStatus 
{
    static const string UNKNOWN = "unknown";
    static const string OK200 = "200";
    static const string MovedPermanently301 = "301";
    static const string BadRequest400 = "400";
    static const string Forbidden403 = "403";
    static const string NotFound404 = "404";
    static const string MethodNotAllow405 = "405";
    static const string InternalServerError500 = "500";
    static const string HttpVersionNotSupported505 = "505";

    static const unordered_map<string, string> STATUS_ {
        {OK200, "OK"}, {MovedPermanently301, "Moved Permanently"}, 
        {BadRequest400, "Bad Request"}, {Forbidden403, "Forbidden"}, 
        {NotFound404, "Not Found"}, {MethodNotAllow405, "Method Not Allowed"},
        {InternalServerError500, "Internal Server Error"}, 
        {HttpVersionNotSupported505, "HTTP Version not supported"}
    };

    static string getStatus(string_view statusCode) 
    {
        string tmp(statusCode);
        if(STATUS_.count(tmp)) {
            return STATUS_.at(tmp);
        }
        return STATUS_.at(InternalServerError500);
    }
};

struct HttpVersion
{
    static const string ErrorVersion = "error";
    static const string Version1_1 = "1.1";

    static const unordered_set<string> versionTable_ {
        Version1_1
    };

    static bool contain(string_view version) 
    {
        string tmp(version);
        if(versionTable_.find(tmp) != versionTable_.end()) {
            return true;
        }
        return false;
    }
};

struct HttpHeaderName 
{
    static const string CONTENT_TYPE = "Content-Type";
    static const string COOKIE = "Cookie";
    static const string SET_COOKIE = "Set-Cookie";
    static const string CONNECTION = "Connection";
    static const string CONTENT_LENGTH = "Content-Length";
    static const string REFRER = "Referer";
    static const string HOST = "Host";
    static const string LOCATION  = "Location";
    static const string SERVER = "Server";
};

struct MIME 
{
    static const string JSON = "application/json; charset=UTF-8";
    static const string MULTIPART_FORM_DATA = "multipart/form-data";
    static const string WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";
    static const string HTML = "text/html; charset=UTF-8";
    static const string TXT = "text/plain; charset=UTF-8";
    static const string JPG = "image/jpeg";
    static const string PNG = "image/png";
    static const string MPEG = "video/mpeg";
    static const string AVI = "video/x-msvideo";
    static const string GZ = "application/x-gzip";
    static const string TAR = "application/x-tar";
    static const string RAR = "application/x-rar-compressed";
    static const string GIF = "image/gif";
    static const string BIN = "application/octet-stream";
    static const string MP4 = "video/mp4";
    static const string MP3 = "audio/x-mpeg";
    static const string PDF = "application/pdf";
    static const string CSS = "text/css";
    static const string JS = "text/javascript";
    static const string XML = "text/xml";

    static const unordered_map<string, string> SUFFIX2TYPE_ {
        {".html", HTML}, {".txt", TXT}, {".jpg", JPG}, {".png", PNG}, 
        {".mp4", MP4}, {".mp3", MP3}, {".avi", AVI}, {".json", JSON}, 
        {".tar", TAR}, {".rar", RAR}, {".gif", GIF}, {".bin", BIN}, 
        {".pdf", PDF}, {".gz", GZ}, {".css", CSS}, {".js", JS},
        {".xml", XML}
    };

    static string getContentTypeBySuffix(string_view suffix) 
    {
        string suf = string(suffix);
        if(SUFFIX2TYPE_.find(suf) != SUFFIX2TYPE_.end()) {
            return SUFFIX2TYPE_.at(suf);
        }
        return TXT;
    }
};

struct HttpErrorHtml
{
    static const unordered_map<string, string> errorHtml_ {
        {HttpStatus::BadRequest400, "/400.html"}, 
        {HttpStatus::Forbidden403, "/403.html"},
        {HttpStatus::NotFound404, "/404.html"}, 
        {HttpStatus::MethodNotAllow405, "/405.html"},
        {HttpStatus::UNKNOWN, "/error.html"}
    };

    static string getErrorHtmlByStatusCode(string_view code) {
        string tmp(code);
        if(errorHtml_.count(tmp)) {
            return errorHtml_.at(tmp);
        }
        return errorHtml_.at(HttpStatus::UNKNOWN);
    }
};

const char * CRLF = "\r\n";

#endif
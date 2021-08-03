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

namespace HttpMethod 
{
    const string UNKNOWN = "unknown";
    const string HEAD = "HEAD";
    const string GET = "GET";
    const string POST = "POST";
    const string PUT = "PUT";
    const string DELETE = "DELETE";

    const unordered_set<string> methodTable_ {
        GET, POST
    };
    
    bool contain(string_view method) 
    {
        string tmp(method);
        if(methodTable_.find(tmp) != methodTable_.end()) {
            return true;
        }
        return false;
    }
}

namespace HttpStatus 
{
    const string UNKNOWN = "unknown";
    const string OK200 = "200";
    const string MovedPermanently301 = "301";
    const string BadRequest400 = "400";
    const string Forbidden403 = "403";
    const string NotFound404 = "404";
    const string MethodNotAllow405 = "405";
    const string InternalServerError500 = "500";
    const string HttpVersionNotSupported505 = "505";

    const unordered_map<string, string> STATUS_ {
        {OK200, "OK"}, {MovedPermanently301, "Moved Permanently"}, 
        {BadRequest400, "Bad Request"}, {Forbidden403, "Forbidden"}, 
        {NotFound404, "Not Found"}, {MethodNotAllow405, "Method Not Allowed"},
        {InternalServerError500, "Internal Server Error"}, 
        {HttpVersionNotSupported505, "HTTP Version not supported"}
    };

    string getStatus(string_view statusCode) 
    {
        string tmp(statusCode);
        if(STATUS_.count(tmp)) {
            return STATUS_.at(tmp);
        }
        return STATUS_.at(InternalServerError500);
    }
}

namespace HttpVersion
{
    const string ErrorVersion = "error";
    const string Version1_1 = "1.1";

    const unordered_set<string> versionTable_ {
        Version1_1
    };

    bool contain(string & version) 
    {
        if(versionTable_.find(version) != versionTable_.end()) {
            return true;
        }
        return false;
    }
}

namespace HttpHeaderName 
{
    const string CONTENT_TYPE = "Content-Type";
    const string COOKIE = "Cookie";
    const string SET_COOKIE = "Set-Cookie";
    const string CONNECTION = "Connection";
    const string CONTENT_LENGTH = "Content-Length";
    const string REFRER = "Referer";
    const string HOST = "Host";
    const string LOCATION  = "Location";
    const string SERVER = "Server";
}

namespace MIME 
{
    const string JSON = "application/json; charset=UTF-8";
    const string MULTIPART_FORM_DATA = "multipart/form-data";
    const string WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";
    const string HTML = "text/html; charset=UTF-8";
    const string TXT = "text/plain; charset=UTF-8";
    const string JPG = "image/jpeg";
    const string PNG = "image/png";
    const string MPEG = "video/mpeg";
    const string AVI = "video/x-msvideo";
    const string GZ = "application/x-gzip";
    const string TAR = "application/x-tar";
    const string RAR = "application/x-rar-compressed";
    const string GIF = "image/gif";
    const string BIN = "application/octet-stream";
    const string MP4 = "video/mp4";
    const string MP3 = "audio/x-mpeg";
    const string PDF = "application/pdf";
    const string CSS = "text/css";
    const string JS = "text/javascript";
    const string XML = "text/xml";

    const unordered_map<string, string> SUFFIX2TYPE_ {
        {".html", HTML}, {".txt", TXT}, {".jpg", JPG}, {".png", PNG}, 
        {".mp4", MP4}, {".mp3", MP3}, {".avi", AVI}, {".json", JSON}, 
        {".tar", TAR}, {".rar", RAR}, {".gif", GIF}, {".bin", BIN}, 
        {".pdf", PDF}, {".gz", GZ}, {".css", CSS}, {".js", JS},
        {".xml", XML}
    };

    string getContentTypeBySuffix(string_view suffix) 
    {
        string suf = string(suffix);
        if(SUFFIX2TYPE_.find(suf) != SUFFIX2TYPE_.end()) {
            return SUFFIX2TYPE_.at(suf);
        }
        return TXT;
    }
}

namespace HttpErrorHtml
{
    const unordered_map<string, string> errorHtml_ {
        {HttpStatus::BadRequest400, "/400.html"}, 
        {HttpStatus::Forbidden403, "/403.html"},
        {HttpStatus::NotFound404, "/404.html"}, 
        {HttpStatus::MethodNotAllow405, "/405.html"},
        {HttpStatus::UNKNOWN, "/error.html"}
    };

    string getErrorHtmlByStatusCode(string_view code) {
        string tmp(code);
        if(errorHtml_.count(tmp)) {
            return errorHtml_.at(tmp);
        }
        return errorHtml_.at(HttpStatus::UNKNOWN);
    }
}

const char * CRLF = "\r\n";

#endif
#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include <unordered_map>
#include <unordered_set>
#include <string>

using std::string;
using std::unordered_map;
using std::unordered_set;

struct HttpMethod {
    static const string UNKNOWN = "unknown";
    static const string HEAD = "HEAD";
    static const string GET = "GET";
    static const string POST = "POST";
    static const string PUT = "PUT";
    static const string DELETE = "DELETE";
    
    static bool contain(string & method) {
        if(methodTable_.find(method) != methodTable_.end()) {
            return true;
        }
        return false;
    }
private:
    static const unordered_set<string> methodTable_ {
        UNKNOWN, HEAD, GET, POST, PUT, DELETE
    };
};

struct HttpStatusCode {
    static const string UNKNOWN = "unknown";
    static const string OK200 = "200";
    static const string MovedPermanently301 = "301";
    static const string BadRequest400 = "400";
    static const string Forbidden403 = "403";
    static const string NotFound404 = "404";
    static const string MethodNotAllow405 = "405";
    static const string InternalServerError500 = "500";
    static const string HttpVersionNotSupported505 = "505";
};

struct HttpVersion {
    static const string ErrorVersion = "error";
    static const string Version1_1 = "1.1";
    static bool contain(string & version) {
        if(versionTable_.find(version) != versionTable_.end()) {
            return true;
        }
        return false;
    }
private:
    static const unordered_set<string> versionTable_ {
        ErrorVersion, Version1_1
    };
};

struct HttpHeaderName {
    static const string CONTENT_TYPE = "Content-Type";
    static const string COOKIE = "Cookie";
    static const string SET_COOKIE = "Set-Cookie";
    static const string CONNECTION = "Connection";
    static const string CONTENT_LENGTH = "Content-Length";
    static const string REFRER = "Referer";
    static const string HOST = "Host";
};

struct MIME {
    static const string JSON = "application/json";
    static const string MULTIPART_FORM_DATA = "multipart/form-data";
    static const string WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";
    static const string HTML = "text/html";
    static const string TXT = "text/plain";
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

    static string getTypeBySuffix(const char * suffix) {
        string suf(suffix);
        if(SUFFIX2TYPE.find() != SUFFIX2TYPE.end()) {
            return SUFFIX2TYPE[suf];
        }
        return "";
    }

    static string getTypeBySuffix(const string & suffix) {
        return getTypeBySuffix(suffix.c_str());
    }
private:
    static const unordered_map<string, string> SUFFIX2TYPE {
        {"html", HTML}, {"txt", TXT}, {"jpg", JPG}, {"png", PNG}, 
        {"mp4", MP4}, {"mp3", MP3}, {"avi", AVI}, {"json", JSON}, 
        {"tar", TAR}, {"rar", RAR}, {"gif", GIF}, {"bin", BIN}, 
        {"pdf", PDF}, {"gz", GZ}, {"css", CSS}, {"js", JS},
        {"xml", XML}
    };

    
};

const std::unordered_set<string> SUPPORTEDMETHOD = {
    "GET", "POST"
};

const std::unordered_set<string> SUPPORTEDVERSION = {
    "1.1"
};

const char * CRLF = "\r\n";

#endif
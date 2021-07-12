#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#include<unordered_map>

enum HttpMethod {
    UNKNOWN,
    HEAD,
    GET,
    POST,
    PUT,
    DELETE
};

enum HttpStatusCode {
    UNKNOWN,
    OK200 = 200,
    MovedPermanently301 = 301,
    BadRequest400 = 400,
    Forbidden403 = 403,
    NotFound404 = 404,
    MethodNotAllow405 = 405,
    InternalServerError500 = 500,
    HttpVersionNotSupported505 = 505
};

enum HttpVersion {
    ErrorVersion,
    Version1_1
};

struct HeaderString {
    static const string CONTENT_TYPE = "content-type";
    static const string COOKIE = "cookie";
    static const string CONNECTION = "connection";
    static const string CONTENT_LENGTH = "content-length";
    static const string REFRER = "referer";
    static const string HOST = "host";
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
};

const std::unordered_map<string, HttpMethod> supportedMethodMap_ = {
    {"GET", GET}, {"POST", POST}, {"PUT", PUT}, {"DELETE", DELETE}
};

const std::unordered_map<string, HttpVersion> supportedHttpVersion_ = {
    {"1.1", Version1_1}
};

const char * CRLF = "\r\n";

#endif
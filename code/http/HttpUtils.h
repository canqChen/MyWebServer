#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#incluide<unordered_map>

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
    ErrorVersion
    Version1_1
};

const std::unordered_map<string, HttpMethod> supportedMethodMap_ = {
    {"GET", GET}, {"POST", POST}, {"PUT", PUT}, {"DELETE", DELETE}
};

const std::unordered_map<string, HttpVersion> supportedHttpVersion_ = {
    {"1.1", Version1_1}
};

const char * CRLF = "\r\n";

#endif
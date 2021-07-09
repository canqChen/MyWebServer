#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

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

#endif
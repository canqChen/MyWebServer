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
    static const string UNKNOWN;
    static const string HEAD;
    static const string GET;
    static const string POST;
    static const string PUT;
    static const string DELETE;

    static const unordered_set<string> methodTable_;
    
    static bool contain(string_view method);
};

struct HttpStatus 
{
    static const string UNKNOWN;
    static const string OK200;
    static const string MovedPermanently301;
    static const string BadRequest400;
    static const string Forbidden403;
    static const string NotFound404;
    static const string MethodNotAllow405;
    static const string InternalServerError500;
    static const string HttpVersionNotSupported505;

    static const unordered_map<string, string> STATUS_;

    static string getStatus(string_view statusCode);
};

struct HttpVersion
{
    static const string ErrorVersion;
    static const string Version1_1;

    static const unordered_set<string> versionTable_;

    static bool contain(string_view version);
};

struct HttpHeaderName 
{
    static const string CONTENT_TYPE;
    static const string COOKIE;
    static const string SET_COOKIE;
    static const string CONNECTION;
    static const string CONTENT_LENGTH;
    static const string REFRER;
    static const string HOST;
    static const string LOCATION;
    static const string SERVER;
};

struct MIME 
{
    static const string JSON;
    static const string MULTIPART_FORM_DATA;
    static const string WWW_FORM_URLENCODED;
    static const string HTML;
    static const string TXT;
    static const string JPG;
    static const string PNG;
    static const string MPEG;
    static const string AVI;
    static const string GZ;
    static const string TAR;
    static const string RAR;
    static const string GIF;
    static const string BIN;
    static const string MP4;
    static const string MP3;
    static const string PDF;
    static const string CSS;
    static const string JS;
    static const string XML;

    static const unordered_map<string, string> SUFFIX2TYPE_;

    static string getContentTypeBySuffix(string_view suffix);
};

struct HttpErrorHtml
{
    static const unordered_map<string, string> errorHtml_;

    static string getErrorHtmlByStatusCode(string_view code);
};

extern const char * CRLF;

#endif
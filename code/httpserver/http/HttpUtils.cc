#include "HttpUtils.h"


const string HttpMethod::UNKNOWN = "unknown";
const string HttpMethod::HEAD = "HEAD";
const string HttpMethod::GET = "GET";
const string HttpMethod::POST = "POST";
const string HttpMethod::PUT = "PUT";
const string HttpMethod::DELETE = "DELETE";

const unordered_set<string> HttpMethod::methodTable_ {
    GET, POST
};

bool HttpMethod::contain(string_view method) 
{
    string tmp(method);
    if(methodTable_.find(tmp) != methodTable_.end()) {
        return true;
    }
    return false;
}

// http status
const string HttpStatus::UNKNOWN = "unknown";
const string HttpStatus::OK200 = "200";
const string HttpStatus::MovedPermanently301 = "301";
const string HttpStatus::BadRequest400 = "400";
const string HttpStatus::Forbidden403 = "403";
const string HttpStatus::NotFound404 = "404";
const string HttpStatus::MethodNotAllow405 = "405";
const string HttpStatus::InternalServerError500 = "500";
const string HttpStatus::HttpVersionNotSupported505 = "505";

const unordered_map<string, string> HttpStatus::STATUS_ {
    {OK200, "OK"}, {MovedPermanently301, "Moved Permanently"}, 
    {BadRequest400, "Bad Request"}, {Forbidden403, "Forbidden"}, 
    {NotFound404, "Not Found"}, {MethodNotAllow405, "Method Not Allowed"},
    {InternalServerError500, "Internal Server Error"}, 
    {HttpVersionNotSupported505, "HTTP Version not supported"}
};

string HttpStatus::getStatus(string_view statusCode) 
{
    string tmp(statusCode);
    if(STATUS_.count(tmp)) {
        return STATUS_.at(tmp);
    }
    return STATUS_.at(InternalServerError500);
}


const string HttpVersion::ErrorVersion = "error";
const string HttpVersion::Version1_1 = "1.1";

const unordered_set<string> HttpVersion::versionTable_ {
    Version1_1
};

bool HttpVersion::contain(string_view version) 
{
    string tmp(version);
    if(versionTable_.find(tmp) != versionTable_.end()) {
        return true;
    }
    return false;
}


const string HttpHeaderName::CONTENT_TYPE = "Content-Type";
const string HttpHeaderName::COOKIE = "Cookie";
const string HttpHeaderName::SET_COOKIE = "Set-Cookie";
const string HttpHeaderName::CONNECTION = "Connection";
const string HttpHeaderName::CONTENT_LENGTH = "Content-Length";
const string HttpHeaderName::REFRER = "Referer";
const string HttpHeaderName::HOST = "Host";
const string HttpHeaderName::LOCATION  = "Location";
const string HttpHeaderName::SERVER = "Server";


const string MIME::JSON = "application/json; charset=UTF-8";
const string MIME::MULTIPART_FORM_DATA = "multipart/form-data";
const string MIME::WWW_FORM_URLENCODED = "application/x-www-form-urlencoded";
const string MIME::HTML = "text/html; charset=UTF-8";
const string MIME::TXT = "text/plain; charset=UTF-8";
const string MIME::JPG = "image/jpeg";
const string MIME::PNG = "image/png";
const string MIME::MPEG = "video/mpeg";
const string MIME::AVI = "video/x-msvideo";
const string MIME::GZ = "application/x-gzip";
const string MIME::TAR = "application/x-tar";
const string MIME::RAR = "application/x-rar-compressed";
const string MIME::GIF = "image/gif";
const string MIME::BIN = "application/octet-stream";
const string MIME::MP4 = "video/mp4";
const string MIME::MP3 = "audio/x-mpeg";
const string MIME::PDF = "application/pdf";
const string MIME::CSS = "text/css";
const string MIME::JS = "text/javascript";
const string MIME::XML = "text/xml";

const unordered_map<string, string> MIME::SUFFIX2TYPE_ {
    {".html", HTML}, {".txt", TXT}, {".jpg", JPG}, {".png", PNG}, 
    {".mp4", MP4}, {".mp3", MP3}, {".avi", AVI}, {".json", JSON}, 
    {".tar", TAR}, {".rar", RAR}, {".gif", GIF}, {".bin", BIN}, 
    {".pdf", PDF}, {".gz", GZ}, {".css", CSS}, {".js", JS},
    {".xml", XML}
};

string MIME::getContentTypeBySuffix(string_view suffix) 
{
    string suf = string(suffix);
    if(SUFFIX2TYPE_.find(suf) != SUFFIX2TYPE_.end()) {
        return SUFFIX2TYPE_.at(suf);
    }
    return TXT;
}


const unordered_map<string, string> HttpErrorHtml::errorHtml_ {
    {HttpStatus::BadRequest400, "/400.html"}, 
    {HttpStatus::Forbidden403, "/403.html"},
    {HttpStatus::NotFound404, "/404.html"}, 
    {HttpStatus::MethodNotAllow405, "/405.html"},
    {HttpStatus::UNKNOWN, "/error.html"}
};

string HttpErrorHtml::getErrorHtmlByStatusCode(string_view code) {
    string tmp(code);
    if(errorHtml_.count(tmp)) {
        return errorHtml_.at(tmp);
    }
    return errorHtml_.at(HttpStatus::UNKNOWN);
}

const char * CRLF = "\r\n";
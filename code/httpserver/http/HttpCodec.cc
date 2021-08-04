#include "HttpCodec.h"

#include <regex>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/types.h>
#include <sys/mman.h>    // mmap, munmap

#include "../utils/StringUtils.h"
#include "HttpUtils.h"
#include "../../common/Buffer.h"
#include "../../common/Log.h"
#include "../utils/URLEncodeUtil.h"
#include "HttpRequest.h"

using std::smatch;
using std::regex;

std::unordered_map<string, std::pair<char *, struct stat> > HttpCodec::cache_;

HttpCodec::HttpCodec() : parseState_(REQUEST_LINE), contextPath_(Config::CONTEXT_PATH){}

HttpRequestPtr HttpCodec::parseHttp(Buffer & buff)
{
    HttpRequestPtr req = std::make_unique<HttpRequest>();
    bool ret = __parseRequest(buff, req);
    // incomplete or wrong message 
    if(!ret) {
        req.reset();
    }
    return req;
}

// parse http, return false if the message is incomplete or error
bool HttpCodec::__parseRequest(Buffer& buff, HttpRequestPtr& req) 
{
    if(buff.readableBytes() <= 0) {
        return false;
    }
    // incomplete message
    if(buff.findDoubleCRLF() == nullptr) {
        return false;
    }
    // for rollback
    auto originBegin = buff.readPtr();

    // start to parse request
    parseState_ = REQUEST_LINE;
    while(buff.readableBytes() > 0 && parseState_ != FINISH) {
        // intercept one line
        const char* lineEnd = buff.findCRLF();
        string line;
        // incomplete massage
        if(lineEnd == nullptr) {
            if(buff.readPtr() != originBegin)
                buff.backwardReadPos(originBegin);
            return false;
        }
        // end of the request line and header
        else if(buff.findCRLF() == buff.readPtr()) {
            buff.forwardReadPos(lineEnd + 2);
            size_t len = req->getContentLength();
            // get request
            if(len == 0) {
                return true;
            }
            else {
                // incomplete message
                if(buff.readableBytes() < len) {
                    buff.backwardReadPos(originBegin);
                    return false;
                }
                parseState_ = BODY;
                line = buff.retrieve(len);
                buff.forwardReadPos(len);
            }
        }
        else {
            // 非请求体
            line = buff.retrieveUtil(lineEnd); // 读取一行
            // 跳过CRLF
            buff.forwardReadPos(lineEnd + 2);
        }

        // 状态机解析请求
        switch(parseState_) {
        case REQUEST_LINE:  
            //  请求行
            if(!__parseRequestLine(line, req)) {
                return false;
            }
            parseState_ = HEADER;
            break;
        case HEADER:       // 头部
            if(!__parseHeader(line, req)) {
                return false;
            }
            break;
        case BODY:          // 请求体
            if(!__parseBody(line, req)) {
                return false;
            }
            parseState_ = FINISH;
            break;
        }
    }
    LOG_DEBUG("[%s], [%s], [%s]", req->requestMethod_.c_str(), \ 
            req->URI_.c_str(), req->requestBody_.c_str());
    return true;
}

bool HttpCodec::__parseRequestLine(const string& line, HttpRequestPtr& req) 
{
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");  // method url http/1.1
    smatch subMatch;
    if(regex_match(line, subMatch, pattern)) {
        string methodStr = subMatch[1];
        if(HttpMethod::contain(methodStr)) {
            req->requestMethod_ = methodStr;
        }
        else {
            req->requestMethod_ = HttpMethod::UNKNOWN;
        }
        req->URL_ = subMatch[2];
        __parseURL(req->URL_, req);
        string verStr(subMatch[3]);
        if(HttpVersion::contain(verStr)) {
            req->httpVersion_ = verStr;
        }
        else {
            req->httpVersion_ = HttpVersion::ErrorVersion;
        }
        LOG_DEBUG("Method: %s, URL: %s, URI: %s, HttpVersion: %s", \
        methodStr.c_str(), req->URL_.c_str(), req->URI_.c_str(), verStr.c_str());
        return true;
    }
    LOG_ERROR("RequestLine Error: %s", line.c_str());
    return false;
}

bool HttpCodec::__parseHeader(const string& line, HttpRequestPtr& req) 
{
    regex patten("^([^:]*): ?(.*)$");  // header: val
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        string sm1(subMatch[1]);
        string sm2(subMatch[2]);
        string lowerHeader = StringUtils::toLower(sm1);
        if(lowerHeader == StringUtils::toLower(HttpHeaderName::COOKIE)) {
            __parseCookie(sm2, req);
        }
        else {
            req->requestHeaders_[lowerHeader] = StringUtils::trim(sm2);
        }
        LOG_DEBUG("header: %s, value: %s", sm1.c_str(), sm2.c_str());
        return true;
    }
    return false;
}

bool HttpCodec::__parseBody(const string& line, HttpRequestPtr& req) 
{
    // record requestbody_
    req->requestBody_ = line;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
    // 解析post请求体
    if(req->requestMethod_ == HttpMethod::POST) {
        __parsePostBody(req->requestBody_, req);
    }
    return true;
}

void HttpCodec::__parseURL(string & url, HttpRequestPtr& req) 
{
    auto decodedUrl = URLEncodeUtils::decode(url);
    regex pattern("^http://([^/]+)/?([^?]*)\\??([^ ]*)");
    smatch subMatch;
    // full url
    if(regex_match(decodedUrl, subMatch, pattern)) {
        string uri = subMatch[2];
        if(uri.empty()) {
            req->URI_ = "/";
        }
        else {
            req->URI_ = uri;
        }
        string params(subMatch[3]);
        if(!params.empty()) {
            __parseParams(params, req);
        }
    }
    else {  // relative url
        auto sep = url.find_first_of('?');
        req->URI_ = url.substr(0, sep);
        // 带参数
        if(sep != string::npos) {
            string params = url.substr(sep + 1);
            __parseParams(params, req);
        }
    }
}

void HttpCodec::__parseCookie(string & cookies, HttpRequestPtr& req) 
{
    auto cookieVec = StringUtils::split(cookies, ";");
    for(auto & cookieStr : cookieVec) {
        cookieStr = StringUtils::trim(cookieStr);
        auto pos = cookieStr.find_first_of('=');
        string key(std::move(cookieStr.substr(0, pos)));
        string val(std::move(cookieStr.substr(pos + 1)));
        req->cookies_[key] = Cookie(key, val);
        LOG_DEBUG("cookie: %s = %s", key.c_str(), val.c_str());
    }
}

// 解析参数，接受urldecode后的参数k-v字符串
void HttpCodec::__parseParams(string & params, HttpRequestPtr& req) 
{
    auto paramsVec = StringUtils::split(params, "&");
    for(auto & paramPair : paramsVec) {
        paramPair = StringUtils::trim(paramPair);
        auto pos = paramPair.find_first_of('=');
        string key = std::move(paramPair.substr(0, pos));
        string val;
        if(pos == paramPair.size() - 1) {
            val = "";
        }
        else {
            val = std::move(paramPair.substr(pos + 1));
        }
        req->requestParameters_[key] = val;
        LOG_DEBUG("param: %s = %s", key.c_str(), val.c_str());
    }
}

void HttpCodec::__parsePostBody(const string & body, HttpRequestPtr& req) 
{
    string contentType = req->requestHeaders_[HttpHeaderName::CONTENT_TYPE];
    
    // parse www-form-urlencoded data
    if(StringUtils::isStartWith(contentType, MIME::WWW_FORM_URLENCODED)) {
        string params = URLEncodeUtils::decode(body, false);
        __parseParams(params, req);
    }
    else if(StringUtils::isStartWith(contentType, MIME::JSON)) {
        // TODO: parse json
    }
    else if(StringUtils::isStartWith(contentType, MIME::MULTIPART_FORM_DATA)) {
        // TODO: parse multipart/form-data
    }
}


// wrap http
void HttpCodec::wrapHttp(const HttpResponsePtr& resp, Buffer & buff) 
{
    __setStatusLine(resp, buff);
    __setHeaders(resp, buff);
    __setBody(resp, buff);
}

void HttpCodec::__setStatusLine(const HttpResponsePtr& resp, Buffer & buff)
{
    string tmp = "HTTP/1.1 " + resp->statusCode_ + " " \
        + HttpStatus::getStatus(resp->statusCode_) + CRLF;
    buff.append(tmp);
}

void HttpCodec::__setHeaders(const HttpResponsePtr& resp, Buffer & buff)
{
    for(auto & [header, value] : resp->headers_) {
        string tmp = header + ": " + value + CRLF;
        buff.append(tmp);
    }
    buff.append(HttpHeaderName::CONNECTION + ": Keep-Alive" + CRLF);
    buff.append(HttpHeaderName::SERVER + ": " + Config::SERVER_NAME + CRLF);
}

void HttpCodec::__setBody(const HttpResponsePtr& resp, Buffer & buff)
{
    if(!StringUtils::isEmpty(resp->fileName_)) {
        string filePath = contextPath_ + resp->fileName_;
        struct stat fileState;
        char * content;
        if(!cache_.count(filePath)) {
            stat(filePath.data(), &fileState);
            int srcFd = open(filePath.data(), O_RDONLY);
            if(srcFd < 0) {
                __setErrorBody(resp, buff, "Internal Server Error");
                LOG_ERROR("Open file %s fail!", filePath.data());
                return;
            }
            LOG_DEBUG("Open file %s", filePath.data());
            // 将文件映射到内存提高文件的访问速度, MAP_PRIVATE 建立一个写入时拷贝的私有映射
            int* mmRet = (int*)mmap(0, fileState.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
            if(*mmRet == -1) {
                __setErrorBody(resp, buff, "Internal Server Error");
                LOG_ERROR("Map file %s fail!", filePath.data());
                return;
            }
            content = (char*)mmRet;
            close(srcFd);
            cache_.insert({filePath, {content, fileState}});
        }
        else {
            content = cache_[filePath].first;
            fileState = cache_[filePath].second;
        }
        buff.append(HttpHeaderName::CONTENT_TYPE + ": " \
            + MIME::getContentTypeBySuffix(StringUtils::getSuffix(filePath)) + CRLF);
        buff.append(HttpHeaderName::CONTENT_LENGTH + ": " + std::to_string(fileState.st_size) + CRLF + CRLF);
        buff.append(content, fileState.st_size);
    }
    else {
        size_t len = resp->contentBuff_.readableBytes();
        buff.append(HttpHeaderName::CONTENT_TYPE + ": " + resp->contentType_ + CRLF);
        buff.append(HttpHeaderName::CONTENT_LENGTH + ": " + std::to_string(len) + CRLF + CRLF);
        buff.append(resp->contentBuff_.readPtr(), len);
    }
}

void HttpCodec::__setErrorBody(const HttpResponsePtr& resp, Buffer& buff, string_view errMessage) 
{
    std::string body;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += resp->statusCode_ + " : " + HttpStatus::getStatus(resp->statusCode_)  + "\n";
    body += "<p>" + std::string(errMessage) + "</p>";
    body += "<hr><em>WebServer by CCQ</em></body></html>";
    buff.append(HttpHeaderName::CONTENT_TYPE + ": " + MIME::HTML + CRLF);
    buff.append(HttpHeaderName::CONTENT_LENGTH + ": " + std::to_string(body.size()) + CRLF + CRLF);
    buff.append(body);
}



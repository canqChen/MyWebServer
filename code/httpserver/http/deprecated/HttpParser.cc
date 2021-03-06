#if 0
#include <regex>
#include "../utils/StringUtils.h"
#include "./HttpParser.h"
#include "./HttpUtils.h"
#include "../../common/Buffer.h"
#include "../../common/Log.h"
#include "../utils/URLEncodeUtil.h"
#include "./HttpRequest.h"

using std::smatch;
using std::regex;

// parse http, return false if the message is incomplete or error
bool HttpParser::__parseRequest(Buffer& buff, std::unique_ptr<HttpRequest>& req) {
    if(buff.readableBytes() <= 0) {
        return false;
    }
    // incomplete message
    if(buff.findDoubleCRLF() == nullptr) {
        return false;
    }
    // for rollback
    auto originBegin = buff.readPtr();
    // FIXME: fix parse process
    // 有限状态机，解析http请求
    parseState_ = REQUEST_LINE;
    while(buff.readableBytes() > 0 && parseState_ != FINISH) {
        // 截取请求的一行
        const char* lineEnd = buff.findCRLF();
        string line;
        // incomplete massage
        if(lineEnd == nullptr) {
            buff.updateReadPos(originBegin);
            return false;
        }
        // end of the request line and header
        else if(buff.findCRLF() == buff.readPtr()) {
            buff.updateReadPos(lineEnd + 2);
            size_t len = req.getContentLength();
            // get request
            if(len == 0) {
                return true;
            }
            else {
                // incomplete message
                if(buff.readableBytes() < len) {
                    buff.updateReadPos(originBegin);
                    return false;
                }
                parseState_ = BODY;
                line = buff.retrieve(len);
                buff.updateReadPos(len);
            }
        }
        else {
            // 非请求体
            line = buff.retrieveUtil(lineEnd); // 读取一行
            // 跳过CRLF
            buff.updateReadPos(lineEnd + 2);
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
            req->requestURI_.c_str(), req->requestBody_.c_str());
    return true;
}

bool HttpParser::__parseRequestLine(const string& line, std::unique_ptr<HttpRequest>& req) {
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
        string verStr = subMatch[3];
        if(HttpVersion::contain(verStr)) {
            req->httpVersion_ = verStr;
        }
        else {
            req->httpVersion_ = HttpVersion::ErrorVersion;
        }
        LOG_DEBUG("Method: %s, URL: %s, HttpVersion: %s", \
        methodStr.c_str(), req->URL_.c_str(), verStr.c_str());
        return true;
    }
    LOG_ERROR("RequestLine Error: %s", line.c_str());
    return false;
}

bool HttpParser::__parseHeader(const string& line, std::unique_ptr<HttpRequest>& req) {
    regex patten("^([^:]*): ?(.*)$");  // header: val
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        string lowerHeader = StringUtils::toLower(subMatch[1]);
        if(lowerHeader == StringUtils::toLower(HttpHeaderName::COOKIE)) {
            __parseCookie(subMatch[2], req);
        }
        else {
            req->requestHeaders_[lowerHeader] = StringUtils::trim(subMatch[2]);
        }
        LOG_DEBUG("header: %s, value: %s", subMatch[1].c_str(), subMatch[2].c_str());
        return true;
    }
    return false;
}

bool HttpParser::__parseBody(const string& line, std::unique_ptr<HttpRequest>& req) {
    // record requestbody_
    requestBody_ = line;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
    // 解析post请求体
    if(requestMethod_ == HttpMethod::POST) {
        __parsePostBody(requestBody_, req);
    }
    return true;
}

void HttpParser::__parseURL(string & url, std::unique_ptr<HttpRequest>& req) {
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
        string params = subMatch[3];
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

void HttpParser::__parseCookie(string & cookies, std::unique_ptr<HttpRequest>& req) {
    auto cookieVec = StringUtils::split(cookies, ";");
    for(auto & cookieStr : cookieVec) {
        cookieStr = StringUtils::trim(cookieStr);
        auto pos = cookieStr.find_first_of('=');
        string key = cookieStr.substr(0, pos);
        string val = cookieStr.substr(pos + 1);
        req->cookies_[key] = val;
        LOG_DEBUG("cookie: %s = %s", key.c_str(), val.c_str());
    }
}

// 解析参数，接受urldecode后的参数k-v字符串
void HttpParser::__parseParams(string & params, std::unique_ptr<HttpRequest>& req) {
    auto paramsVec = StringUtils::split(params, "&");
    for(auto & paramPair : paramsVec) {
        paramPair = StringUtils::trim(paramPair);
        auto pos = paramPair.find_first_of('=');
        string key = paramPair.substr(0, pos);
        string val = paramPair.substr(pos + 1);
        req->requestParameters_[key] = val;
        LOG_DEBUG("param: %s = %s", key.c_str(), val.c_str());
    }
}

void HttpParser::__parsePostBody(const string & body, std::unique_ptr<HttpRequest>& req) {
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
#endif
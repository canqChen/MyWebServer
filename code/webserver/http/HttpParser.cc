#include <regex>
#include "../utils/StringUtils.h"
#include "./HttpParser.h"
#include "./HttpUtils.h"
#include "../../common/Buffer.h"
#include "../../common/Log.h"
#include "../utils/URLEncodeUtil.h"

using std::smatch;
using std::regex;

// 解析http请求
void HttpParser::__parseRequest(Buffer& buff, std::unique_ptr<HttpRequest>& req) {
    if(buff.readableBytes() <= 0) {
        return false;
    }
    // 有限状态机，解析http请求
    parseState_ = REQUEST_LINE;
    while(buff.readableBytes() > 0 && parseState_ != FINISH) {
        // 截取请求的一行
        const char* lineEnd = buff.findCRLF();
        string line;
        if(lineEnd == nullptr) {
            // 读取剩余请求体部分
            line = buff.retrieveAll();
            // 应该与content-length字节数一致
            assert(static_cast<int>(line.size()) \
                == stoi(req->requestHeaders_[HeaderString::CONTENT_LENGTH]))
        }
        else {
            // 非请求体
            line = buff.retrieveUtil(lineEnd); // 读取一行
            // 跳过CRLF
            buff.updateReadPos(lineEnd + 2);
        }

        // 状态机解析请求
        switch(parseState_) {
        case REQUEST_LINE:  //  请求行
            if(!__parseRequestLine(line, req)) {
                return;
            }
            parseState_ = HEADER;
            break;
        case HEADER:       // 头部
            if(!__parseHeader(line, req)) {
                parseState_ = BODY;
            }
            break;
        case BODY:          // 请求体
            if(!__parseBody(line, req)) {
                req->statusCode_ = BadRequest400;
                return;
            }
            parseState_ = FINISH;
            break;
        }
    }
    LOG_DEBUG("[%s], [%s], [%s]", req->requestMethod_.c_str(), \ 
            req->requestURI_.c_str(), req->requestBody_.c_str());
}

bool HttpParser::__parseRequestLine(const string& line, std::unique_ptr<HttpRequest>& req) {
    regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");  // method url http/1.1
    smatch subMatch;
    if(regex_match(line, subMatch, pattern)) {
        string methodStr = subMatch[1];
        if(supportedMethodMap_.count(methodStr) > 0) {
            req->requestMethod_ = supportedMethodMap_.at(methodStr);
        }
        else { // 请求方法不支持
            req->requestMethod_ = UNKNOWN;
            req->statusCode_ = MethodNotAllow405;
            return false;
        }
        req->URL_ = subMatch[2];
        string verStr = subMatch[3];
        if(supportedHttpVersion_.count(verStr)) {
            req->httpVersion_ = supportedHttpVersion_.at(verStr);
        }
        else {
            req->httpVersion_ = ErrorVersion;
            req->statusCode_ = HttpVersionNotSupported505;
            return false;
        }
        LOG_DEBUG("Method: %s, URL: %s, HttpVersion: %s", \
        methodStr.c_str(), req->URL_.c_str(), verStr.c_str());
        return true;
    }
    LOG_ERROR("RequestLine Error: %s", line.c_str());
    req->statusCode_ = BadRequest400;    // 请求行解析失败，请求有误
    return false;
}

bool HttpParser::__parseHeader(const string& line, std::unique_ptr<HttpRequest>& req) {
    regex patten("^([^:]*): ?(.*)$");  // opt: val
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        string lowerHeader = StringUtils::toLower(subMatch[1]);
        if(lowerHeader == HeaderString::COOKIE) {
            __parseCookie(subMatch[2], req);
        }
        else {
            req->requestHeaders_[lowerHeader] = StringUtils::toLower(subMatch[2]);
        }
        LOG_DEBUG("header: %s, value: %s", subMatch[1].c_str(), subMatch[2].c_str());
        return true;
    }
    // 匹配失败，遇到空行，状态到解parsebody
    return false;
}

bool HttpParser::__parseBody(const string& line, std::unique_ptr<HttpRequest>& req) {
    requestBody_ = line;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
    // 解析post请求体
    if(requestMethod_ == HttpMethod::POST) {
        __parsePostBody(requestBody_, req);
    }
    return true;
}

void HttpParser::__parseURL(string & url, std::unique_ptr<HttpRequest>& req) {
    if(requestMethod_ == HttpMethod::GET) {
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
    // 解析表单数据
    string contentType = req->requestHeaders_[HeaderString::CONTENT_TYPE];
    if(contentType == MIME::WWW_FORM_URLENCODED) {
        // post body中编码与url不一样
        string params = URLEncodeUtils::decode(body, false);
        __parseParams(params, req);
    }
    else if(StringUtils::startWith(contentType, MIME::JSON)) {
        // TODO: 解析json
    }
    else if(StringUtils::startWith(contentType, MIME::MULTIPART_FORM_DATA)) {
        // TODO: 解析multipart/form-data
    }
}
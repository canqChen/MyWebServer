
#include "HttpRequest.h"
using namespace std;

void HttpRequest::init() {
    requestURI_ = httpVersion_ = requestBody_ = "";
    requestMethod_ = HttpMethod::UNKNOWN;
    statusCode_ = HttpStatusCode::OK200;
    parseState_ = REQUEST_LINE;
    requestHeaders_.clear();
    requestParameters_.clear();
}

bool HttpRequest::isKeepAlive() const {
    if(requestHeaders_.count("Connection") == 1) {
        return requestHeaders_.at("Connection") == "keep-alive" && httpVersion_ == "1.1";
    }
    return false;
}

// 解析http请求
void HttpRequest::parseRequest(Buffer& buff) {
    if(buff.readableBytes() <= 0) {
        return false;
    }
    // 有限状态机，解析http请求
    parseState_ = REQUEST_LINE;
    while(buff.readableBytes() > 0 && parseState_ != FINISH) {
        // 截取请求的一行
        const char* lineEnd = buff.findCRLF();  
        string line = buff.retrieveUtil(lineEnd); // 读取一行
        // 状态机解析请求
        switch(parseState_)
        {
        case REQUEST_LINE:  //  请求行
            if(!__parseRequestLine(line)) {
                return;
            }
            parseState_ = HEADER;
            break;
        case HEADER:       // 头部
            if(!__parseHeader(line)) // 返回false，解析完毕，进入下一状态
                parseState_ = BODY;
            if(buff.readableBytes() <= 2) { // get请求，到达末尾，没有请求体
                return;
            }
            break;
        case BODY:          // 请求体
            __parseBody(line);
            parseState_ = FINISH;
            break;
        default: // FINISH
            break;
        }
        if(lineEnd == buff.writePtr()) { // 末尾
            break;
        }
        // 跳过CRLF
        buff.updateReadPos(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", requestMethod_.c_str(), requestURI_.c_str(), requestMethod_.c_str());
    return true;
}

bool HttpRequest::__parseRequestLine(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");  // method url http/1.1
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        string methodStr = subMatch[1];
        if(supportedMethodMap_.count(methodStr) > 0) {
            requestMethod_ = supportedMethodMap_.at(methodStr);
        }
        else { // 请求方法不支持
            requestMethod_ = UNKNOWN;
            statusCode_ = MethodNotAllow405;
            return false;
        }
        requestURL_ = subMatch[2];
        requestMethod_ = subMatch[3];
        return true;
    }
    LOG_ERROR("RequestLine Error: %s", line.c_str());
    statusCode_ = BadRequest400;    // 请求行解析失败，请求有误
    return false;
}

bool HttpRequest::__parseHeader(const string& line) {
    regex patten("^([^:]*): ?(.*)$");  // opt: val
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        requestHeaders_[subMatch[1]] = subMatch[2];
        LOG_DEBUG("header: %s, value: %s", sub_match[1].c_str(), sub_match[2].c_str());
        return true;
    }
    // 匹配失败，遇到空行，状态到解析body
    return false;
}

bool HttpRequest::__parseBody(const string& line) {
    requestBody_ = line;
    __parsePost();    // post方法处理
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::__parseGetParameters() {
    if(requestMethod_ == HttpMethod::GET) {

    }
}

void HttpRequest::__parsePostBody() {
    if(requestMethod_ == HttpMethod::POST && requestHeaders_["Content-Type"] == "application/x-www-form-urlencoded") {
        __parseFromUrlencoded();
        if(DEFAULT_HTML_TAG.count(requestURI_)) {   // 仅register.html 和 login.html 支持post方法
            int tag = DEFAULT_HTML_TAG.at(requestURI_);
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(requestParameters_["username"], requestParameters_["password"], isLogin)) {   // 验证用户密码是否正确
                    requestURI_ = "/welcome.html";
                }
                else {
                    requestURI_ = "/error.html";
                }
            }
        }
    }
}

/*
编码：用于键值对参数，参数之间用&间隔, 如果有空格，将空格转换为+加号；=号前是key；有特殊符号，用%标记，并将特殊符号转换为ASCII HEX值

1. 字母数字字符 "a" 到 "z"、"A" 到 "Z" 和 "0" 到 "9" 保持不变。

2. 特殊字符 "."、"-"、"*" 和 "_" 保持不变。

3. 空格字符 " " 转换为一个加号 "+"

4. 所有其他字符都是不安全的，因此首先使用一些编码机制将它们转换为一个或多个字节。然后每个字节用一个包含 3 个字符的字符串 "%xy" 表示，
其中 xy 为该字节的两位十六进制表示形式。编码机制是 UTF-8。
*/
void HttpRequest::__parseFromUrlencoded() {
    if(requestBody_.size() == 0) { return; }
    string key, value;
    int num = 0;
    int n = requestBody_.size();
    int i = 0, j = 0;

    auto convert2Hex = [](char ch)->int {
        if(ch >= 'A' && ch <= 'F') 
            return ch -'A' + 10;
        if(ch >= 'a' && ch <= 'f') 
            return ch -'a' + 10;
        return ch;
    };

    for(; i < n; i++) {
        char ch = requestBody_[i];
        switch (ch) {
        case '=':
            key = requestBody_.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            requestBody_[i] = ' ';
            break;
        case '%':
            num = convert2Hex(requestBody_[i + 1]) * 16 + convert2Hex(requestBody_[i + 2]);
            requestBody_[i + 2] = num % 10 + '0';
            requestBody_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = requestBody_.substr(j, i - j);
            j = i + 1;
            requestParameters_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(requestParameters_.count(key) == 0 && j < i) {
        value = requestBody_.substr(j, i - j);
        requestParameters_[key] = value;
    }
}

bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql,  SqlConnPool::GetInstance());
    assert(sql);
    
    bool flag = false;
    unsigned int j = 0;
    char order[256] = { '\0' };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!isLogin) { flag = true; }
    // 查询用户及密码
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1",  name.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        string password(row[1]);
        
        if(isLogin) {       // 登录
            if(pwd == password) 
            { 
                flag = true; 
            }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } 
        else {      // 注册，用户名已被使用
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    // 注册行为 且 用户名未被使用
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')",  name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    SqlConnPool::GetInstance()->FreeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

string HttpRequest::getRequestURI() const{
    return requestURI_;
}

HttpMethod HttpRequest::getMethod() const {
    return requestMethod_;
}

string HttpRequest::getHttpVersion() const {
    return requestMethod_;
}

string HttpRequest::getParameter(const string & name) const {
    assert(key != "");
    if(requestParameters_.count(name) == 1) {
        return requestParameters_.at(name);
    }
    return "";
}

string HttpRequest::getParameter(const char* name) const {
    if(name == nullptr)
        return "";
    string key(name);
    return getParameter(key);
}

#include "HttpRequest.h"
using namespace std;

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
            "/welcome", "/video", "/picture", };

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init() {
    mMethod = mResourcePath = mVersion = mBody = "";
    mState = REQUEST_LINE;
    mHeader.clear();
    mPost.clear();
}

bool HttpRequest::IsKeepAlive() const {
    if(mHeader.count("Connection") == 1) {
        return mHeader["Connection"] == "keep-alive" && mVersion == "1.1";
    }
    return false;
}

// 解析http请求
bool HttpRequest::Parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    // 有限状态机，解析http请求
    while(buff.ReadableBytes() && mState != FINISH) {
        const char* lineEnd = search(buff.ReadBeginPointer(), buff.NextWriteBeginPointerConst(), CRLF, CRLF + 2);  // 读取一行
        std::string line(buff.ReadBeginPointer(), lineEnd);
        switch(mState)
        {
        case REQUEST_LINE:  //  请求行
            if(!ParseRequestLine(line)) {
                return false;
            }
            ParsePath();
            break;    
        case HEADERS:       // 头部
            ParseHeader(line);
            if(buff.ReadableBytes() <= 2) {
                mState = FINISH;
            }
            break;
        case BODY:          // 主体
            ParseBody(line);
            break;
        default:
            break;
        }

        if(lineEnd == buff.NextWriteBeginPointer()) { 
            break; 
        }
        buff.UpdateReadUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", mMethod.c_str(), mResourcePath.c_str(), mVersion.c_str());
    return true;
}

void HttpRequest::ParsePath() {
    if(mResourcePath == "/") {          // 默认路径
        mResourcePath = "/index.html"; 
    }
    else {
        for(auto &item: DEFAULT_HTML) {     // 指定文件
            if(item == mResourcePath) {
                mResourcePath += ".html";
                break;
            }
        }
    }
}

bool HttpRequest::ParseRequestLine(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");  // method url http/1.1
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {   
        mMethod = subMatch[1];
        mResourcePath = subMatch[2];
        mVersion = subMatch[3];
        mState = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader(const string& line) {
    regex patten("^([^:]*): ?(.*)$");  // opt: val
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        mHeader[subMatch[1]] = subMatch[2];
    }
    else {
        mState = BODY;
    }
}

void HttpRequest::ParseBody(const string& line) {
    mBody = line;
    ParsePost();    // post方法处理
    mState = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpRequest::ParsePost() {
    if(mMethod == "POST" && mHeader["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded();
        if(DEFAULT_HTML_TAG.count(mResourcePath)) {   // 仅register.html 和 login.html 支持post方法
            int tag = DEFAULT_HTML_TAG[mResourcePath];
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(mPost["username"], mPost["password"], isLogin)) {   // 验证用户密码是否正确
                    mResourcePath = "/welcome.html";
                }
                else {
                    mResourcePath = "/error.html";
                }
            }
        }
    }   
}

// 编码：用于键值对参数，参数之间用&间隔, 如果有空格，将空格转换为+加号；=号前是key；有特殊符号，用%标记，并将特殊符号转换为ASCII HEX值
void HttpRequest::ParseFromUrlencoded() {
    if(mBody.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = mBody.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = mBody[i];
        switch (ch) {
        case '=':
            key = mBody.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            mBody[i] = ' ';
            break;
        case '%':
            num = ConverHex(mBody[i + 1]) * 16 + ConverHex(mBody[i + 2]);
            mBody[i + 2] = num % 10 + '0';
            mBody[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = mBody.substr(j, i - j);
            j = i + 1;
            mPost[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(mPost.count(key) == 0 && j < i) {
        value = mBody.substr(j, i - j);
        mPost[key] = value;
    }
}

bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);
    
    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!isLogin) { flag = true; }
    // 查询用户及密码
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
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
        // 注册行为 且 用户名未被使用
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } 
        else { 
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    // 注册行为 且 用户名未被使用
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
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

std::string HttpRequest::Path() const{
    return mResourcePath;
}

std::string& HttpRequest::Path(){
    return mResourcePath;
}
std::string HttpRequest::Method() const {
    return mMethod;
}

std::string HttpRequest::Version() const {
    return mVersion;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(mPost.count(key) == 1) {
        return mPost[key];
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(mPost.count(key) == 1) {
        return mPost[key];
    }
    return "";
}
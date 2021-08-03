
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>

#include "../../common/Log.h"
#include "HttpUtils.h"
#include "Cookie.h"
#include "../../common/Buffer.h"

using std::string;
using std::string_view;

class HttpCodec;

class HttpResponse : NoCopyable
{
public:
    friend class HttpCodec;
    HttpResponse();
    ~HttpResponse() = default;

    void sendError(string_view sc)
    {
        statusCode_ = sc;
        fileName_ = HttpErrorHtml::getErrorHtmlByStatusCode(statusCode_);
    }

    void sendRedirect(string_view url)
    {
        statusCode_ = HttpStatus::MovedPermanently301;
        setHeader(HttpHeaderName::LOCATION, url);
    }

    void setHeader(string_view header, string_view value)
    {
        headers_.insert({header, value});
    }

    void addContent(string_view content)
    {
        contentBuff_.append(content.data(), content.size());
    }

    void addCookie(string_view name, string_view value)
    {
        Cookie ck(name, value);
        headers_.insert({HttpHeaderName::SET_COOKIE, ck.getCookieStr()});
    }

    void addCookie(const Cookie & ck)
    {
        headers_.insert({HttpHeaderName::SET_COOKIE, ck.getCookieStr()});
    }

    void setContentType(string_view type) 
    {
        headers_.insert({HttpHeaderName::CONTENT_TYPE, type});
    }

    void setFileAsContent(string_view fileName)
    {
        fileName_ = fileName;
    }

private:
    string statusCode_;
    string contentType_;
    string fileName_;
    Buffer contentBuff_;
    std::multimap<string, string> headers_;
};


#endif //HTTPRESPONSE_H
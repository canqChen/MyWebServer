#include "../interceptor/CheckHttpValidInterceptor.h"
#include "../http/HttpUtils.h"

bool CheckHttpValidInterceptor::doIntercept(const HttpRequestPtr &req, HttpResponsePtr&resp)
{
    if(!HttpVersion::contain(req->getHttpVersion())) {
        resp->sendError(HttpStatus::HttpVersionNotSupported505);
        return false;
    }
    if(!HttpMethod::contain(req->getRequestMethod())) {
        resp->sendError(HttpStatus::MethodNotAllow405);
        return false;
    }
    return true;
}
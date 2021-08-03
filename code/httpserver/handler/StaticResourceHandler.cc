#include <unistd.h>
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../handler/StaticResourceHandler.h"
#include "../Config.h"


void StaticResourceHandler::doGet(const HttpRequestPtr &req, HttpResponsePtr & resp) 
{
    string path = Config::CONTEXT_PATH + req->getRequestURI();
    if(access(path.c_str(), F_OK | R_OK) == 0) {
        resp->setFileAsContent(req->getRequestURI());
    }
    else {
        resp->sendError(HttpStatus::NotFound404);
    }
}
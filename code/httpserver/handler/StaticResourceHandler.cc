#include <unistd.h>
#include "httpserver/http/HttpRequest.h"
#include "httpserver/http/HttpResponse.h"
#include "httpserver/handler/StaticResourceHandler.h"
#include "httpserver/Config.h"


void StaticResourceHandler::doGet(const HttpRequestPtr &req, HttpResponsePtr & resp) 
{
    string path = Config::CONTEXT_PATH + req->getRequestURI();
    if(access(path, F_OK | R_OK) == 0) {
        resp->setFileAsContent(req->getRequestURI());
    }
    else {
        resp->sendError(HttpStatus::NotFound404);
    }
}
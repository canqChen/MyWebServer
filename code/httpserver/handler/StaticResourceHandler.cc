#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include "../handler/StaticResourceHandler.h"
#include "../Config.h"


void StaticResourceHandler::doGet(const HttpRequestPtr &req, HttpResponsePtr & resp) 
{
    string resourceName = req->getRequestURI();
    string suffix = StringUtils::getSuffix(resourceName);
    if(resourceName == "/") {
        resourceName = "/index.html";
    }
    else if(StringUtils::isEmpty(suffix)) {
        resourceName += ".html";
    }
    string path = Config::CONTEXT_PATH + resourceName;
    LOG_DEBUG("resource path: %s", path.c_str());
    struct stat buf;
    lstat(path.c_str(), &buf);
    if(access(path.c_str(), F_OK | R_OK) == 0 && S_ISREG(buf.st_mode)) {
        resp->setFileAsContent(resourceName);
    }
    else {
        resp->sendError(HttpStatus::NotFound404);
    }
}
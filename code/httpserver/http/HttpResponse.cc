#include "httpserver/http/HttpResponse.h"
#include "httpserver/Config.h"

HttpResponse::HttpResponse()
    : statusCode_(HttpStatus::OK200), fileName_(""), 
      contentType_(MIME::TXT), contentBuff_(512ull, 8ull)
{}



#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/Buffer.h"
#include "../log/Log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void init(const std::string& srcDir, std::string& resourcePath, bool isKeepAlive = false, int code = -1);
    void makeResponse(Buffer& buff);
    void unmapFile();
    char* getFile();
    size_t fileSize() const;
    void errorContent(Buffer& buff, std::string message);
    int Code() const { return mCode; }

    void addStateLine(Buffer &buff);
    void setHeader(Buffer &buff);
    void addContent(Buffer &buff);
    void setCookie(string name, string value);
    void setContentType(String type);

private:
    void ErrorHtml();
    std::string GetFileType();

    int mCode;
    bool mIsKeepAlive;

    std::string mResourcePath;
    std::string mSrcDir;
    
    char* mMmFile; 
    struct stat mMmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_TO_STATUS;
    static const std::unordered_map<int, std::string> ERRCODE_TO_PATH;
};


#endif //HTTP_RESPONSE_H
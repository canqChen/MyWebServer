#ifndef CONFIG_H
#define CONFIG_H

#include <string>

using std::string;

namespace Config 
{
    const string CONTEXT_PATH = "../resources";
    const uint16_t SERVER_PORT = 1314;
    const uint32_t LOOPS = 6;
    const uint32_t WORKERS = 6;
    const string SERVER_NAME = "WebServer by CCQ";
}

#endif
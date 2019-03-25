// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"
#include <getopt.h>
#include <string>
#include <charon/mvm>
#include <charon/base>

using mvm = charon::mvm;

int main(int argc, char *argv[])
{
    mvm::init(argc, argv);
    int threadNum = 4;
    int port = 80;
    std::string logPath = "/linya_WebServer.log";

    // parse args
    int opt;
    const char *str = "t:l:p:";
    while ((opt = getopt(argc, argv, str))!= -1)
    {
        switch (opt)
        {
            case 't':
            {
                threadNum = atoi(optarg);
                break;
            }
            case 'l':
            {
                logPath = optarg;
                if (logPath.size() < 2 || optarg[0] != '/')
                {
                    printf("logPath should start with \"/\"\n");
                    abort();
                }
                break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default: break;
        }
    }
    Logger::setLogFileName(logPath);
    /*
    LOG << os::abspath("app/services");
    if( os::exists(os::abspath("app/services")) ) {
      LOG << "iniciando services";
      charon::service::load(os::abspath("app/services"));
    } else {
      LOG <<  "nao tem diretorio app/services";
    }
    */

    // STL库在多线程上应用
    #ifndef _PTHREADS
        LOG << "_PTHREADS is not defined !";
    #endif
    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, threadNum, port);
    myHTTPServer.start();
    mainLoop.loop();
    return 0;
}

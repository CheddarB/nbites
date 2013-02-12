#pragma once

#include "RoboGrams.h"
#include "Logger.h"
#include <string>

namespace man {
namespace log {

static std::string FILEPATH = "/home/nao/nbites/frames/";
static std::string EXT = ".frm";

class LogModule : public portals::Module {

public:
	LogModule();
	virtual ~LogModule() {}

    template<class T>
    void addLogger(portals::OutPortal<T>* port, std::string name)
    {
        logs.push_back(new Logger<T>(port, name));
    }

protected:
    void run_();
    std::vector<LoggerBase*> logs;
};

}
}

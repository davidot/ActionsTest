#include "Process.h"

#ifndef WINDOWS_PROCESS
#error Only for windows process handling
#endif

namespace util {

    SubProcess::~SubProcess() {
        stop();
    }

    bool SubProcess::writeTo(std::string_view) const {
        return false;
    }

    bool SubProcess::readLine(std::string&) const {
        return false;
    }

    SubProcess::ProcessExit SubProcess::stop() {
        if (running) {
            running = false;
        }
        return {};
    }

    std::unique_ptr<SubProcess> SubProcess::create(std::vector<std::string>) {
        return nullptr;
    }

}
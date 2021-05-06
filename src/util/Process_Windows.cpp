#include "Process.h"

#ifndef WINDOWS_PROCESS
#error Only for windows process handling
#endif

namespace util {

    SubProcess::~SubProcess() {
        stop();
    }

    bool writeTo(std::string_view str) {

    }

    bool readLine(std::string& line) {

    }

    ProcessExit stop() {

    }

    static std::unique_ptr<SubProcess> SubProcess::create(std::vector<std::string> command) {
        return nullptr;
    }

}
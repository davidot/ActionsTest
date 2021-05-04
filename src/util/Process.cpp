#include "Process.h"

SubProcess::SubProcess(std::string_view commandLine) {
}

bool SubProcess::write(std::string_view) {
    return false;
}

bool SubProcess::read(std::string&) {
    return false;
}

void SubProcess::stop() {
}

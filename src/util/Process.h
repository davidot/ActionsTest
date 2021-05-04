#pragma once

#include <string_view>

class SubProcess {
public:
    SubProcess(std::string_view commandLine);

    bool write(std::string_view);

    bool read(std::string&);

    void stop();
};
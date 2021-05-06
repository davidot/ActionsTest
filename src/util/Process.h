#pragma once

#include <string_view>
#include <vector>
#include <optional>
#include <memory>

#if !defined(POSIX_PROCESS) && !defined(WINDOWS_PROCESS)
#error Must define one of POSIX_PROCESS or WINDOWS_PROCESS
#endif

namespace util {

    class SubProcess {
    public:
        ~SubProcess();

        static std::unique_ptr<SubProcess> create(std::vector<std::string> command);

        bool writeTo(std::string_view) const;

        bool readLine(std::string&) const;

        struct ProcessExit {
            bool stopped = false;
            std::optional<int> exitCode;
        };

        ProcessExit stop();
    private:
        bool running = true;

#ifdef POSIX_PROCESS
        pid_t m_procPid;

        mutable std::vector<char> readBuffer = std::vector<char>(4096lu, '\0');
        mutable ssize_t m_bufferLoc = 0;

        int m_stdIn = -1;
        int m_stdOut = -1;

        std::optional<int> m_exitCode;

        bool readLineFromBuffer(std::string&) const;
#elif defined(WINDOWS_PROCESS)

#endif
    };

}
#pragma once

#include <string_view>
#include <vector>
#include <optional>
#include <memory>

#ifdef POSIX_PROCESS
#elif defined(WINDOWS_PROCESS)
#include <windows.h>
#else
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

        mutable std::vector<char> readBuffer = std::vector<char>(4096lu, '\0');
        mutable int32_t m_bufferLoc = 0;
        bool readLineFromBuffer(std::string&) const;

#ifdef POSIX_PROCESS
        pid_t m_procPid;

        int m_stdIn = -1;
        int m_stdOut = -1;

        std::optional<int> m_exitCode;
#elif defined(WINDOWS_PROCESS)
        HANDLE m_childProc;

        HANDLE m_stdIn;
        HANDLE m_stdOut;

        // Do not use before stop is called (i.e. running = true)
        ProcessExit exitState{};
#endif
    };

}

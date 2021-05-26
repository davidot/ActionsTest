#include "Process.h"
#include "Assertions.h"

#ifndef WINDOWS_PROCESS
#error Only for windows process handling
#endif

#include <iostream>
#include <numeric>
#include <algorithm>

namespace util {

    void outputError(const std::string& operation) {
        auto errorID = GetLastError();
        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                     nullptr, errorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);
        std::string errorMessage(messageBuffer, size);
        std::cerr << operation << " failed with: [" << errorID << "] " << errorMessage;
    }

#define TRY_OR_FAIL(operation, onError) \
    if (!(operation)) {                 \
        outputError(#operation);        \
        onError                                \
    }



    SubProcess::~SubProcess() {
        stop();
    }

    bool SubProcess::writeTo(std::string_view str) const {
        char const* head = str.data();
        DWORD toWrite = str.size();

        while (toWrite > 0) {
            DWORD written;
            if (!WriteFile(m_stdIn, head, toWrite, &written, nullptr)) {
                outputError("WriteFile");
                return false;
            }
            toWrite -= written;
            head += toWrite;
        }

        return true;
    }

    bool SubProcess::readLine(std::string& line) const {
        while (!readLineFromBuffer(line)) {
            DWORD readBytes;
            if (!ReadFile(m_stdOut, readBuffer.data() + m_bufferLoc, readBuffer.size() - m_bufferLoc,
                          &readBytes, nullptr)) {
                outputError("ReadFile");
                return false;
            }
            auto readAmount = static_cast<int32_t>(readBytes);
            auto end = std::remove(readBuffer.begin() + m_bufferLoc, readBuffer.begin() + m_bufferLoc + readAmount, '\r');
            m_bufferLoc = std::distance(readBuffer.begin(), end);
        }
        return true;

    }

    SubProcess::ProcessExit SubProcess::stop() {
        if (running) {
            running = false;

            CloseHandle(m_stdIn);
            CloseHandle(m_stdOut);

            DWORD waited = WaitForSingleObject(m_childProc, 1000);
            switch (waited) {
                case WAIT_OBJECT_0:
                    // shutdown
                    exitState.stopped = true;
                    DWORD code;
                    if (!GetExitCodeProcess(m_childProc, &code)) {
                        outputError("GetExitCodeProcess");
                    } else {
                        if (code == STILL_ACTIVE) {
                            exitState.stopped = false;
                            std::cerr << "Process stopped but not stopped?\n";
                        } else {
                            exitState.exitCode = code;
                        }
                    }
                    break;
                case WAIT_TIMEOUT:
                    // Try to terminate
                    if (!TerminateProcess(m_childProc, 0)) {
                        outputError("TerminateProcess");
                    } else {
                        exitState.stopped = true;
                    }
                    break;
                case WAIT_FAILED:
                    outputError("WaitForSingleObject");
                    break;
                default:
                    ASSERT_NOT_REACHED();
                    break;
            }

            CloseHandle(m_childProc);
        }
        return exitState;
    }

    std::unique_ptr<SubProcess> SubProcess::create(std::vector<std::string> command) {

#define ON_ERROR_SHOW_AND_FAIL(call) TRY_OR_FAIL(call, return nullptr;)

        HANDLE childStdinRead;
        HANDLE childStdinWrite;
        HANDLE childStdoutRead;
        HANDLE childStdoutWrite;

        SECURITY_ATTRIBUTES securityAttributes = {
                .nLength = sizeof(securityAttributes),
                .lpSecurityDescriptor = nullptr,
                .bInheritHandle = true,
        };

        ON_ERROR_SHOW_AND_FAIL(CreatePipe(&childStdoutRead, &childStdoutWrite, &securityAttributes, 0))
        ON_ERROR_SHOW_AND_FAIL(SetHandleInformation(childStdoutRead, HANDLE_FLAG_INHERIT, 0))

        ON_ERROR_SHOW_AND_FAIL(CreatePipe(&childStdinRead, &childStdinWrite, &securityAttributes, 0))
        ON_ERROR_SHOW_AND_FAIL(SetHandleInformation(childStdinWrite, HANDLE_FLAG_INHERIT, 0))


        PROCESS_INFORMATION childInfo;
        STARTUPINFO startupInfo;
        ZeroMemory(&childInfo, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

        startupInfo.cb = sizeof(STARTUPINFO);
        startupInfo.hStdOutput = childStdoutWrite;
        startupInfo.hStdInput = childStdinRead;
        startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        startupInfo.dwFlags |= STARTF_USESTDHANDLES;

        ASSERT(!command.empty());

        auto quoteIfSpaces = [](const std::string& str) {
            if (auto space = std::find(str.begin(), str.end(), ' '); space != str.end() &&
                    str.front() != '"') {
                return '"' + str + '"';
            }
            return str;
        };

        std::string program = quoteIfSpaces(command.front());

        std::string fullCommand = std::accumulate(std::next(command.begin()), command.end(), program,
              [&quoteIfSpaces](const std::string& acc, const std::string& rhs){
                            if (rhs.empty()) {
                                return acc;
                            }
                            return acc + ' ' + quoteIfSpaces(rhs);
                        });

        if (!CreateProcess(nullptr, (LPSTR)fullCommand.c_str(),
                             nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, nullptr,
                             &startupInfo, &childInfo)) {
            outputError("Create process with: _" + fullCommand + "_");
            return nullptr;
        }

        CloseHandle(childInfo.hThread);
        CloseHandle(childStdoutWrite);
        CloseHandle(childStdinRead);

        auto process = std::make_unique<SubProcess>();
        process->m_stdIn = childStdinWrite;
        process->m_stdOut = childStdoutRead;
        process->m_childProc = childInfo.hProcess;
        process->running = true;

        return process;
    }

}

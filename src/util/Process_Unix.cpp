#include "Process.h"

#ifndef POSIX_PROCESS
#error Only for windows process handling
#endif

#include "Assertions.h"
#include "StringUtil.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

namespace util {

    SubProcess::~SubProcess() {
        stop();
    }

    bool SubProcess::writeTo(std::string_view str) const {
        char const* head = str.data();
        auto toWrite = static_cast<ssize_t>(str.size());

        while (toWrite > 0) {
            ssize_t written = write(m_stdIn, head, toWrite);
            if (written < 0) {
                perror("write");
                return false;
            }
            toWrite -= written;
            head += toWrite;
        }

        return true;
    }

    bool SubProcess::readLineFromBuffer(std::string& line) const {
        if (m_bufferLoc <= 0) {
            return false;
        }
        auto bufferEnd = readBuffer.begin() + m_bufferLoc + 1;
        auto newLine = std::find(readBuffer.begin(), bufferEnd, '\n');

        if (newLine == bufferEnd) {
            return false;
        }

        // include newline in message
        ++newLine;

        line = std::string(readBuffer.begin(), newLine);

        std::fill(readBuffer.begin(), newLine, '\0');

        // optimization if newLine was the last real character skip rotating
        if (std::distance(readBuffer.begin(), newLine) <= m_bufferLoc) {
            std::rotate(readBuffer.begin(), newLine, readBuffer.end());
        }

        return true;
    }

    bool SubProcess::readLine(std::string& line) const {
        if (readLineFromBuffer(line)) {
            return true;
        }

        ssize_t readBytes = read(m_stdOut, readBuffer.data() + m_bufferLoc, readBuffer.size() - m_bufferLoc);
        if (readBytes < 0) {
            perror("read");
            return false;
        }
        m_bufferLoc += readBytes;

        return readLineFromBuffer(line);
    }

    SubProcess::ProcessExit SubProcess::stop() {
        if (running) {
            running = false;

            // should trigger command ending
            close(m_stdIn);

            int status;
            if (pid_t waited = waitpid(m_procPid, &status, 0); waited < 0) {
                perror("waitpid");
            }
            close(m_stdOut);

            if (!WIFEXITED(status)) {
                std::cerr << "Child was stopped non normally?\n";
            } else {
                int stat = WEXITSTATUS(status);
                if (stat != 0) {
                    std::cerr << "Process exited with non-zero: " << stat << '\n';
                }

                m_exitCode = stat;
            }

        }

        return {true, m_exitCode};
    }

    constexpr int pipeRead = 0;
    constexpr int pipeWrite = 1;
    constexpr int maxCommandSize = 64;

    std::unique_ptr<SubProcess> SubProcess::create(std::vector<std::string> command) {
        if (command.size() >= maxCommandSize || command.empty()) {
            ASSERT_NOT_REACHED();
            return nullptr;
        }

        char* args[maxCommandSize];
        int i = 0;
        for (auto& sv : command) {
            args[i++] = sv.data();
        }
        args[i] = nullptr;

        int inPipe[2] = {-1, -1};
        if (pipe(inPipe) < 0) {
            perror("pipe");
            return nullptr;
        }

#define CLOSE_PIPE(pipe) \
    close((pipe)[0]); \
    close((pipe)[1])

        int outPipe[2] = {-1, -1};
        if (pipe(outPipe) < 0) {
            perror("pipe");
            CLOSE_PIPE(inPipe);
            return nullptr;
        }

        int startPipe[2] = {-1, -1};
        if (pipe(startPipe) < 0) {
            perror("pipe");
            CLOSE_PIPE(inPipe);
            CLOSE_PIPE(outPipe);
            return nullptr;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            CLOSE_PIPE(inPipe);
            CLOSE_PIPE(outPipe);
            ASSERT_NOT_REACHED();

            return nullptr;
        } else if (pid > 0) {
            auto proc = std::make_unique<SubProcess>();
            proc->m_procPid = pid;
            proc->m_stdIn = inPipe[pipeWrite];
            proc->m_stdOut = outPipe[pipeRead];

            close(inPipe[pipeRead]);
            close(outPipe[pipeWrite]);
            close(startPipe[pipeWrite]);

            char buf[9] = {};

            ssize_t readStart = read(startPipe[pipeRead], buf, 8);

            if (readStart != 0) {
                if (readStart < 0) {
                    perror("read");
                    std::cerr << "Read fail!\n";
                } else {
                    std::cerr << "Failed to start: " << command[0] << '\n'
                              << "Got: _" << buf << "_\n";
                }

                close(startPipe[pipeRead]);

                close(inPipe[pipeWrite]);
                close(outPipe[pipeRead]);
                proc->running = false;

                return nullptr;
            }

            close(startPipe[pipeRead]);

            return proc;
        }

        close(inPipe[pipeWrite]);
        close(outPipe[pipeRead]);
        close(startPipe[pipeRead]);

        dup2(inPipe[pipeRead], STDIN_FILENO);
        dup2(outPipe[pipeWrite], STDOUT_FILENO);

        fcntl(startPipe[pipeWrite], F_SETFD, FD_CLOEXEC);

        // child
        if (execvp(command[0].c_str(), args) < 0) {
            perror("execvp");
            char buf[] = "fail";
            write(startPipe[pipeWrite], buf, 4);
            std::exit(-4);
        }
        ASSERT_NOT_REACHED();
        std::exit(0);
        return {};
    }

}

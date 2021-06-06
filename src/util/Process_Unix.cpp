#include "Process.h"

#ifndef POSIX_PROCESS
#error Only for posix process handling
#endif

#include "Assertions.h"
#include "StringUtil.h"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#ifdef __APPLE__
extern char **environ;
#endif

namespace util {

    SubProcess::~SubProcess() {
        stop();
    }

    bool SubProcess::writeTo(std::string_view str) const {
        if (!running) {
            return false;
        }
        char const* head = str.data();
        auto toWrite = static_cast<ssize_t>(str.size());

        struct sigaction act{};
        struct sigaction oldAct{};
        act.sa_flags = 0;
        act.sa_handler = SIG_IGN;
        if (sigaction(SIGPIPE, &act, &oldAct) < 0) {
            perror("sigaction");
            return false;
        }

        // TODO: add some scope exit convenience maybe?
        auto recoverSigaction = [&oldAct]() {
          if (sigaction(SIGPIPE, &oldAct, nullptr) < 0) {
              perror("sigaction");
          }
        };

        while (toWrite > 0) {
            ssize_t written = write(m_stdIn, head, toWrite);
            if (written < 0) {
                if (errno == EPIPE) {
                    running = false;
                } else {
                    perror("write");
                }
                recoverSigaction();
                return false;
            }
            toWrite -= written;
            head += toWrite;
        }

        recoverSigaction();
        return true;
    }

    bool SubProcess::readLine(std::string& line) const {
        if (!running) {
            return false;
        }
        while (!readLineFromBuffer(line)) {
            ssize_t readBytes = read(m_stdOut, readBuffer.data() + m_bufferLoc, readBuffer.size() - m_bufferLoc);
            if (readBytes < 0) {
                perror("read");
                return false;
            }
            if (readBytes == 0) {
                return false;
            }
            m_bufferLoc += readBytes;
        }
        return true;
    }

    SubProcess::ProcessExit SubProcess::stop() {
        if (!m_waitCalled) {
            running = false;

            // should trigger command ending
            close(m_stdIn);

            int status;
            if (pid_t waited = waitpid(m_procPid, &status, 0); waited < 0) {
                perror("waitpid");
            }
            close(m_stdOut);

            m_waitCalled = true;

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

        posix_spawn_file_actions_t actions;
        if (posix_spawn_file_actions_init(&actions)) {
            perror("posix_spawn_file_actions_init");
            return nullptr;
        }

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

        // Setup child operations
        posix_spawn_file_actions_addclose(&actions, inPipe[pipeWrite]);
        posix_spawn_file_actions_addclose(&actions, outPipe[pipeRead]);

        posix_spawn_file_actions_adddup2(&actions, inPipe[pipeRead], STDIN_FILENO);
        posix_spawn_file_actions_adddup2(&actions, outPipe[pipeWrite], STDOUT_FILENO);

        pid_t pid;
        if (posix_spawn(&pid, command[0].c_str(), &actions, nullptr, args, environ)) {
            perror("posix_spawn");
            std::cout << command[0] << '\n';
            CLOSE_PIPE(inPipe);
            CLOSE_PIPE(outPipe);
            posix_spawn_file_actions_destroy(&actions);
            return nullptr;
        }

        close(inPipe[pipeRead]);
        close(outPipe[pipeWrite]);

        auto proc = std::make_unique<SubProcess>();
        proc->m_procPid = pid;
        proc->m_stdIn = inPipe[pipeWrite];
        proc->m_stdOut = outPipe[pipeRead];

        proc->running = true;

        if (posix_spawn_file_actions_destroy(&actions)) {
            perror("posix_spawn_file_actions_destroy");
            // uh oh we just started it... try to stop and otherwise whatever
            ASSERT_NOT_REACHED();
            proc->stop();
            return nullptr;
        }

        return proc;
    }

}

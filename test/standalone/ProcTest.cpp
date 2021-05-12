#include <functional>
#include <iostream>
#include <string>
#include <util/Process.h>

using util::SubProcess;

static bool passed = true;
static std::string testAppLocation;

static std::string currentTest;

static std::vector<std::function<void()>> tests;


#define TEST_NAME(x) test_##x
#define TEST_NAME_INTERNAL(x) test_perf##x
#define TEST_STRUCT(x) str_##x

#define TEST_CASE(name)                              \
    static void TEST_NAME(name)();                   \
    struct TEST_STRUCT(name) {                       \
        TEST_STRUCT(name)                            \
        () noexcept {                                         \
            tests.emplace_back(TEST_NAME(name));        \
        }                                            \
    };                                               \
    static TEST_STRUCT(name) TEST_STRUCT(name);      \
    static bool TEST_NAME_INTERNAL(name)();          \
    static void TEST_NAME(name)() {                  \
        currentTest = #name;                         \
        if (!TEST_NAME_INTERNAL(name)()) {           \
            passed = false;                          \
            std::cout << "Failed test: " #name "\n"; \
            std::cerr << "Failed test: " #name "\n"; \
        } else {                                     \
            std::cout << "Test " #name " passed\n";  \
        }                                            \
        std::cerr.flush();                                             \
        std::cout.flush();                                             \
    }                                                \
    static bool TEST_NAME_INTERNAL(name)()

#define EXPECT(expr) \
    if (!(expr)) {   \
        std::cerr << "Expected: " #expr " but failed in" << currentTest << ' ' << __FILE__ << ':' << __LINE__ << '\n'; \
        return false;\
    }

#define EXPECT_EQ(lhs, rhs) \
    if (!((lhs) == (rhs))) {    \
        std::cerr << "Expected: " #lhs " == " #rhs " in " \
                  << currentTest << ' ' << __FILE__ << ':' << __LINE__ \
                  << "\nBut got   _" << (lhs) << "_ != _" << (rhs) << "_\n";\
        return false;\
    }


#define EXIT_STATUS(cond) \
    auto exit = proc->stop(); \
    EXPECT(exit.stopped);        \
    EXPECT(exit.exitCode cond);\
    return true

#define EXIT_ZERO() EXIT_STATUS(== 0)

#define EXIT_FAILED() EXIT_STATUS(!= 0)

TEST_CASE(NonExistant) {
    auto proc = SubProcess::create({"ShouldAbsolutelyNotExist"});
    EXPECT(!proc);
    return true;
}

TEST_CASE(StartAndStop) {
    auto proc = SubProcess::create({testAppLocation});
    EXPECT(proc);

    auto exit = proc->stop();
    EXPECT(exit.stopped);
    EXPECT(exit.exitCode == 0);
    return true;
}

TEST_CASE(ReadSingleLine) {
    auto proc = SubProcess::create({testAppLocation, "--write"});
    EXPECT(proc);
    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "write\n");

    EXPECT(!proc->readLine(line));

    EXIT_ZERO();
}

TEST_CASE(ReadNoTextRead) {
    auto proc = SubProcess::create({testAppLocation, "--write"});
    EXPECT(proc);

    EXIT_ZERO();
}

TEST_CASE(ReadMultipleLines) {
    auto proc = SubProcess::create({testAppLocation, "--write-x"});
    EXPECT(proc);
    std::string line;
    for (int i = 0; i < 10; ++i) {
        EXPECT(proc->readLine(line));
        EXPECT_EQ(line, "write\n");
    }

    EXPECT(!proc->readLine(line));

    EXIT_ZERO();
}

TEST_CASE(ReadLineWithSpaces) {
    std::string arg = "This string has spaces!";
    auto proc = SubProcess::create({testAppLocation, "--write", arg});
    EXPECT(proc);
    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, arg + "\n");

    EXPECT(!proc->readLine(line));

    EXIT_ZERO();
}

TEST_CASE(WriteTextNoEndLine) {
    auto proc = SubProcess::create({testAppLocation, "--read"});
    EXPECT(proc);

    EXPECT(proc->writeTo("write"));

    EXIT_ZERO();
}

TEST_CASE(WriteMultipleLinesAtOnce) {
    auto proc = SubProcess::create({testAppLocation, "--read"});
    EXPECT(proc);

    EXPECT(proc->writeTo("w\nr\ni\nt\ne\n"));

    EXIT_ZERO();
}

TEST_CASE(WriteNoTextCauseFailure) {
    auto proc = SubProcess::create({testAppLocation, "--read"});
    EXPECT(proc);

    EXIT_FAILED();
}

TEST_CASE(WriteReadLoopWithQuit) {
    auto proc = SubProcess::create({testAppLocation, "--continue-until-quit"});
    EXPECT(proc);

    EXPECT(proc->writeTo("ping\n"));
    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "pong\n");

    EXPECT(proc->writeTo("quit\n"));

    EXIT_ZERO();
}

TEST_CASE(MultipleWriteReads) {
    auto proc = SubProcess::create({testAppLocation, "--continue-until-quit"});
    EXPECT(proc);

    EXPECT(proc->writeTo("ping\n"));
    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "pong\n");

    EXPECT(proc->writeTo("nice\n"));

    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "noce\n");

    EXIT_FAILED();
}

TEST_CASE(WriteReadLoopNoQuit) {
    auto proc = SubProcess::create({testAppLocation, "--continue-until-quit"});
    EXPECT(proc);

    EXPECT(proc->writeTo("ping\n"));
    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "pong\n");

    EXIT_FAILED();
}

TEST_CASE(CanReadExitCode) {
    for (int i : {0, 1, 12, 243}) {
        EXPECT(i >= 0);
        auto proc = SubProcess::create({testAppLocation, "--exit-code", std::to_string(i)});
        EXPECT(proc);

        auto exit = proc->stop();
        EXPECT(exit.stopped);
        EXPECT(exit.exitCode == i);
    }
    return true;
}

TEST_CASE(SlowReadWaitsUntilLine) {
    auto proc = SubProcess::create({testAppLocation, "--slow-write"});

    std::string line;
    EXPECT(proc->readLine(line));
    EXPECT_EQ(line, "write\n");

    EXPECT(!proc->readLine(line));

    EXIT_ZERO();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Give location of test app\n";
        return -1;
    }

    testAppLocation = argv[1];
    std::cerr << "Test app at: " << argv[1] << std::endl;

    bool failFast = false;
    if (argc > 2) {
        std::string flag = argv[2];
        if (flag == "-f" || flag == "--fail-fast") {
            failFast = true;
        }
    }

    for (auto& t : tests) {
        t();
        if (failFast && !passed) {
            std::cout << "Stopping at first test failure!\n";
            break;
        }
    }

    return passed ? 0 : 1;
}

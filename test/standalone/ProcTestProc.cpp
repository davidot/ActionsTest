#include <iostream>
#include <string>

int read() {
    std::string word;
    std::cin >> word;
    if (!std::cin) {
        std::cout << "Empty\n";
        return 1;
    } else {
        std::cout << "Read: " << word << '\n';
        return 0;
    }
}

int readLine() {
    std::string line;
    if (std::getline(std::cin, line)) {
        std::cout << "Read line: " << line << '\n';
        return 0;
    } else {
        std::cout << "Empty\n";
        return 1;
    }
}

int continueUntilQuit() {
    std::string line;
    while(std::getline(std::cin, line)) {
        if (line.back() == '\n') {
            std::cerr << "Get line with line end!\n";
            return 1;
        }
        if (line == "quit") {
            break;
        }
        // ping -> pong
        // win -> won
        std::replace(line.begin(), line.end(), 'i', 'o');
        std::cout << line << '\n';
        std::cout.flush();
    }
    if (!std::cin) {
        return 1;
    }

    return 0;
}

int write(const char* str, int times = 1) {
    for (int i =0 ; i < times; ++i) {
        std::cout << str << '\n';
    }
    return 0;
}

int argToInt(char* str, int defaultVal) {
    char* end;
    int code = static_cast<int>(strtol(str, &end, 10));
    if (end == str) {
        std::cerr << "Could not read digit\n";
        return defaultVal;
    }
    return code;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        // no argument just quit
        return 0;
    }

    std::string arg = argv[1];
    if (arg == "--exit-code") {
        int code = 1;
        if (argc > 2) {
            code = argToInt(argv[2], 1);
        }
        return code;
    } else if (arg == "--read") {
        return read();
    } else if (arg == "--readline") {
        return readLine();
    } else if (arg == "--continue-until-quit") {
        return continueUntilQuit();
    } else if (arg == "--write") {
        if (argc > 2) {
            return write(argv[2]);
        }
        return write("write");
    } else if (arg == "--write-x") {
        int times = 10;
        if (argc > 2) {
            times = argToInt(argv[2], 10);
        }
        write("write", times);
    } else {
        std::cerr << "Unknown argument" << argv[2] << '\n';
        return 1;
    }

    return 0;
}

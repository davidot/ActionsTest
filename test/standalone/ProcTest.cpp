#include <iostream>
#include <string>
#include <util/Process.h>


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Give location of test app\n";
        return -1;
    }

    std::cout << "Running test app at: " << argv[1] << '\n';
    SubProcess proc{argv[1]};

    return 0;
}
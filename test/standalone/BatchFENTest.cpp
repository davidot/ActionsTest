#include <chess/Board.h>
#include <iostream>
#include <fstream>
#include <string>

int main(int argv, char** argc) {
    if (argv <= 1) {
        std::cerr << "Use like " << argc[0] << " [options] <filename>\n";
        return 1;
    }

    bool showHelp = false;
    bool quiet = false;
    bool keepGoing = false;
    std::string fileName;

    for (int i = 1; i < argv; i++) {
        std::string arg = argc[i];
        if (arg.empty()) {
            std::cerr << "Empty arg? " << i << '\n';
            continue;
        }
        if (arg[0] == '-') {
            if (arg == "-h" || arg == "--help" || arg == "-?" || arg == "\\?") {
                showHelp = true;
                break;
            } else if (arg == "-q" || arg == "--quiet") {
                quiet = true;
            } else if (arg == "-c" || arg == "--continue") {
                keepGoing = true;
            }
        } else {
            fileName = arg;
        }
    }

    if (showHelp) {
        std::cerr << argc[0] << ":"
                  << " Read all FENs from file line by line\n"
                  << "Use like " << argc[0] << " [options] <filename>\n"
                  << "Options: \n"
                  << "   -q, --quiet     Silence all output except invalid FENs and failures \n"
                  << "   -h, --help      Show this help message\n"
                  << "   -c, --continue  Don't stop after first non valid FEN or failure (failures might not be detectable)\n"
                ;

        return 0;
    }

    if (!quiet) {
        std::cout << "Reading from " << fileName << '\n';
    }

    uint64_t readLines = 0;
    uint64_t failure = 0;
    uint64_t overDraw = 0;

    std::ifstream file(fileName);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            Chess::ExpectedBoard eb = Chess::Board::fromFEN(line);
            if (!eb) {
                ++failure;
                std::cerr << "Could not parse _" << line << "_" << '\n'
                    <<       "      With error: " << eb.error() << '\n';
                if (!keepGoing) {
                    return 3;
                }
            } else if (eb.value().halfMovesSinceIrreversible() >= 150) {
                ++overDraw;
            }
            ++readLines;
        }

        file.close();
    } else {
        std::cerr << "Could not open file: " << fileName << '\n';
        return 2;
    }

    std::cout << "Read " << readLines << " FENs with " << failure << " failures\n";
    std::cout << overDraw << " position which are forced draws\n";

    return 0;
}
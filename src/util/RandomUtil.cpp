#include "RandomUtil.h"
#include "Assertions.h"
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <type_traits>

namespace util {

    using seed_int = std::random_device::result_type;
    constexpr size_t hex_per_int = sizeof(seed_int) * 2;
    constexpr size_t int_per_row = 8;

    std::string outputSeed(const std::vector<uint32_t>& values) {
        std::ostringstream stream;
        stream << std::hex << std::setfill('0');

        constexpr const char* spacing = "";

        stream << spacing;

        for (size_t i = 0; i < values.size(); i++) {
            stream << std::setw(hex_per_int) << values[i];

            if ((i + 1) >= values.size()) {
                break;
            }

            if (i % int_per_row == int_per_row - 1) {
                // was last in row
                stream << '\n' << spacing;
            } else {
                stream << ' ';
            }
        }

        return stream.str();
    }


    std::pair<seed_vector, std::string> generateSeed(size_t bytes) {
        ASSERT(bytes % sizeof(seed_int) == 0);
        const size_t ints = bytes / sizeof(seed_int);
        std::vector<uint32_t> values(ints);
        {
            std::random_device randomDevice;
            for (size_t i = 0; i < ints; i++) {
                values[i] = randomDevice();
            }
        }

        return {values, outputSeed(values)};
    }

    std::optional<seed_vector> loadSeed(const std::string& mnemonic, size_t bytes) {
        ASSERT(bytes % sizeof(seed_int) == 0);
        const size_t ints = bytes / sizeof(seed_int);
        std::istringstream inputString(mnemonic);
        inputString >> std::skipws >> std::hex;

        std::vector<uint32_t> values(ints);

        for (size_t i = 0; i < ints; i++) {
            inputString >> std::setw(hex_per_int) >> values[i];
            if (!inputString) {
                return std::nullopt;
            }
        }

        return values;
    }
}

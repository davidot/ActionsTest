#include "StringUtil.h"


namespace util {
    std::vector<std::string_view> split(std::string_view vw, std::string_view separator) {
        std::vector<std::string_view> parts{};
        auto last = vw.begin();
        auto nextFront = std::search(vw.begin(), vw.end(), separator.begin(), separator.end());
        while (nextFront != vw.end()) {
            parts.emplace_back(last, nextFront);
            last = nextFront;
            std::advance(last, separator.size());
            nextFront = std::search(last, vw.end(), separator.begin(), separator.end());
        }

        parts.emplace_back(last, vw.end());

        return parts;
    }

}

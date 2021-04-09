#include "StringUtil.h"
#include <algorithm>
#include <vector>


namespace util {
    std::vector<std::string_view> split(std::string_view vw, std::string_view separator) {
        std::vector<std::string_view> parts{};
        if (separator.empty()) {
            for (size_t i = 0; i < vw.size(); i++) {
                parts.push_back(vw.substr(i, 1));
            }
            if (vw.empty()) {
                parts.emplace_back();
            }
            return parts;
        }

        auto begin = vw.begin();
        auto last = vw.begin();
        auto nextFront = std::search(vw.begin(), vw.end(), separator.begin(), separator.end());
        while (nextFront != vw.end()) {
            parts.push_back(vw.substr(std::distance(begin, last), std::distance(last, nextFront)));
            last = nextFront;
            std::advance(last, separator.size());
            nextFront = std::search(last, vw.end(), separator.begin(), separator.end());
        }

        parts.push_back(vw.substr(std::distance(begin, last), std::distance(last, vw.end())));

        return parts;
    }

}

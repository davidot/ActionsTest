#pragma once
#include <string_view>
#include <vector>

namespace util {

    std::vector<std::string_view> split(std::string_view vw, std::string_view separator);

}
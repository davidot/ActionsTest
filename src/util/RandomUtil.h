#pragma once

#include <optional>
#include <tuple>
#include <random>

namespace util {

    using seed_vector = std::vector<std::random_device::result_type>;

    std::pair<seed_vector, std::string> generateSeed(size_t bytes);

    std::optional<seed_vector> loadSeed(const std::string& mnemonic, size_t bytes);

    template<typename RNG>
    RNG seedRNG(std::string* textOutput = nullptr, size_t bytes = RNG::state_size * sizeof(RNG::word_size)) {
        auto [seed, mnemonic] = generateSeed(bytes);

        if (textOutput != nullptr) {
            *textOutput = std::move(mnemonic);
        }

        std::seed_seq seq(seed.begin(), seed.end());
        return RNG(seq);
    }

    template<typename RNG>
    std::optional<RNG> seedRNGFromString(std::string& textOutput, size_t bytes = RNG::state_size * sizeof(RNG::word_size)) {
        seed_vector seed;

        if (textOutput.empty()) {
            auto [seedVal, mnemonic] = generateSeed(bytes);
            textOutput = std::move(mnemonic);
            seed = std::move(seedVal);
        } else {
            auto opt = loadSeed(textOutput, bytes);
            if (!opt) {
                return std::nullopt;
            }
            seed = opt.value();
        }

        std::seed_seq seq(seed.begin(), seed.end());
        return RNG(seq);
    }



}


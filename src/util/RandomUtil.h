#pragma once

#include <random>
namespace util {

    template<typename RNG>
    RNG seedRNG() {
        std::random_device source;

        std::vector<std::random_device::result_type> bits(RNG::word_size);
        std::generate(std::begin(bits), std::end(bits), [&](){return source();});

        std::seed_seq seeds(std::begin(bits), std::end(bits));
        RNG rng (seeds);
        return rng;
    }

}


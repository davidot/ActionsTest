#pragma once

#include <catch2/catch.hpp>
#include <chess/Forward.h>
#include <chess/Piece.h>
#include <vector>
#ifndef EXTENDED_TESTS
#define TEST_SOME(x) sample(2, x)
#else
#define TEST_SOME(x) x
#endif


namespace Catch::Generators {

    template<typename T>
    class SamplingGenerator final : public IGenerator<T> {
        std::vector<T> m_values;

    public:
        SamplingGenerator(size_t target, GeneratorWrapper<T> &&generator) : SamplingGenerator(target, std::numeric_limits<size_t>::max(), std::move(generator)) {};

        SamplingGenerator(size_t target, size_t generatorSize, GeneratorWrapper<T> &&generator) {
            assert(target != 0 && "Empty generators are not allowed");
            size_t generated = 0;
            m_values.reserve(target);
            for (; generated < target; ++generated) {
                m_values.push_back(generator.get());
                if (!generator.next()) {
                    // ran out of values already
                    return;
                }
            }

            Catch::SimplePcg32& random = rng();
            do {
                auto j = std::uniform_int_distribution<size_t>(0, generated)(random);
                if (j < target) {
                    m_values[j] = generator.get();
                }
                ++generated;
            } while (generated < generatorSize && generator.next());
            if (generated == generatorSize && generatorSize >= 1000 * target) {
                Catch::throw_exception(GeneratorException("You probably want to limit the sample size"));
            }
        }

        T const &get() const override {
            return m_values.back();
        }

        bool next() override {
            if (!m_values.empty()) {
                m_values.pop_back();
            }
            return !m_values.empty();
        }
    };

    template<typename T>
    GeneratorWrapper<T> sample(size_t target, GeneratorWrapper<T> &&generator) {
        return GeneratorWrapper<T>(pf::make_unique<SamplingGenerator<T>>(target, std::move(generator)));
    }

    template<typename T>
    GeneratorWrapper<T> sample(size_t target, size_t totalSize, GeneratorWrapper<T> &&generator) {
        return GeneratorWrapper<T>(pf::make_unique<SamplingGenerator<T>>(target, totalSize, std::move(generator)));
    }

}// namespace Catch::Generators

namespace TestUtil {
    Chess::Board generateCastlingBoard(Chess::Color toMove, bool kingSide, bool queenSide, bool withOppositeRook);

    Chess::Board createEnPassantBoard(Chess::Color c, Chess::BoardIndex col);
}

namespace std { // NOLINT(cert-dcl58-cpp) This just really helps for logging!
    std::ostream &operator<<(std::ostream &os, const std::optional<Chess::Piece> &piece);
}
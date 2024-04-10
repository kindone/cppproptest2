#pragma once
#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/std/string.hpp"

/**
 * @file string.hpp
 * @brief Arbitrary for string
 */
namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for string with configurable character generators and min/max sizes
 */
template <>
class PROPTEST_API Arbi<string> final : public ArbiContainer<string> {
    using ArbiContainer<string>::minSize;
    using ArbiContainer<string>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);
    Arbi(Generator<char> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);

    Shrinkable<string> operator()(Random& rand) const override;

private:
    Generator<char> elemGen;
};

}  // namespace proptest

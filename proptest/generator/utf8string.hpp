#pragma once
#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/util/utf8string.hpp"

/**
 * @file utf8string.hpp
 * @brief Arbitrary for UTF8String
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for UTF-8 string with configurable code generator and min/max sizes
 */
template <>
class PROPTEST_API Arbi<UTF8String> final : public ArbiContainer<UTF8String> {
public:
    using ArbiContainer<UTF8String>::minSize;
    using ArbiContainer<UTF8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);
    Arbi(GenFunction<uint32_t> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);

    Shrinkable<UTF8String> operator()(Random& rand) const override;

private:
    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest

#pragma once
#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/util/utf16string.hpp"

/**
 * @file utf16string.hpp
 * @brief Arbitrary for UTF16BEString and UTF16LEString
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for UTF-16 big endian string with configurable code generator and min/max sizes
 */
template <>
class PROPTEST_API Arbi<UTF16BEString> final : public ArbiContainer<UTF16BEString> {
public:
    using ArbiContainer<UTF16BEString>::minSize;
    using ArbiContainer<UTF16BEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);
    Arbi(GenFunction<uint32_t> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);

    Shrinkable<UTF16BEString> operator()(Random& rand) const override;

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<Arbi>(elemGen, minSize, maxSize);
    }

private:
    GenFunction<uint32_t> elemGen;
};

/**
 * @ingroup Generators
 * @brief Arbitrary for UTF-16 little endian string with configurable code generator and min/max sizes
 */
template <>
class PROPTEST_API Arbi<UTF16LEString> final : public ArbiContainer<UTF16LEString> {
public:
    using ArbiContainer<UTF16LEString>::minSize;
    using ArbiContainer<UTF16LEString>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);
    Arbi(GenFunction<uint32_t> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);

    Shrinkable<UTF16LEString> operator()(Random& rand) const override;

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<Arbi>(elemGen, minSize, maxSize);
    }

private:
    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest

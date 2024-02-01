#pragma once
#include "proptest/api.hpp"
#include "proptest/Arbitrary.hpp"
#include "proptest/util/cesu8string.hpp"

/**
 * @file cesu8string.hpp
 * @brief Arbitrary for CESU8String
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for CESU-8 string with configurable code generator and min/max sizes
 */
template <>
class PROPTEST_API Arbi<CESU8String> final : public ArbiContainer<CESU8String> {
public:
    using ArbiContainer<CESU8String>::minSize;
    using ArbiContainer<CESU8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);
    Arbi(GenFunction<uint32_t> _elemGen,size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize);

    Shrinkable<CESU8String> operator()(Random& rand) const override;

    shared_ptr<GeneratorBase> clone() const override {
        return util::make_shared<Arbi>(elemGen, minSize, maxSize);
    }

private:
    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest

#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/vector.hpp"


/**
 * @file vector.hpp
 * @brief Arbitrary for vector<T>
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for vector<T> with configurable element generators and min/max sizes
 */
template <typename T>
class PROPTEST_API Arbi<vector<T>> final : public ArbiContainer<vector<T>> {
public:
    using Vector = vector<T>;
    using ArbiContainer<vector<T>>::minSize;
    using ArbiContainer<vector<T>>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<vector<T>>(_minSize, _maxSize), elemGen(Arbi<T>()) {}

    Arbi(GenFunction<T> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<vector<T>>(_minSize, _maxSize), elemGen(_elemGen) {}

    Arbi setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Shrinkable<vector<T>> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkVec = make_shrinkable<vector<ShrinkableAny>>();
        shrinkVec.getMutableRef().reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec.getMutableRef().push_back(elemGen(rand));

        return shrinkListLike<vector, T>(shrinkVec, minSize);
    }

    shared_ptr<GeneratorBase<vector<T>>> clone() const override {
        return util::make_shared<Arbi>(elemGen, minSize, maxSize);
    }

private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<vector<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<vector<T>>::defaultMaxSize = 200;

}  // namespace proptest

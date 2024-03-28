#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/list.hpp"
#include "proptest/std/vector.hpp"

/**
 * @file list.hpp
 * @brief Arbitrary for list<T>
 */
namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for list<T> with configurable element generator and min/max sizes
 */
template <typename T>
class PROPTEST_API Arbi<list<T>> final : public ArbiContainer<list<T>> {
public:
    using List = list<T>;
    using ArbiContainer<List>::minSize;
    using ArbiContainer<List>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<list<T>>(_minSize, _maxSize), elemGen(Arbi<T>()) {}

    Arbi(GenFunction<T> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<list<T>>(_minSize, _maxSize), elemGen(_elemGen) {}

    Arbi setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Shrinkable<list<T>> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkVec = make_shrinkable<vector<ShrinkableAny>>();
        shrinkVec.getMutableRef<vector<ShrinkableAny>>().reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec.getMutableRef<vector<ShrinkableAny>>().push_back(elemGen(rand));

        return shrinkListLike<list, T>(shrinkVec, minSize);
    }

private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<list<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<list<T>>::defaultMaxSize = 200;

}  // namespace proptest

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

    Arbi() : ArbiContainer<List>(defaultMinSize, defaultMaxSize), elemGen(Arbi<T>()) {}

    Arbi(Arbi<T>& _elemGen)
        : ArbiContainer<List>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) mutable -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbi(GenFunction<T> _elemGen) : ArbiContainer<List>(defaultMinSize, defaultMaxSize), elemGen(_elemGen) {}

    Shrinkable<list<T>> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkVec = make_shrinkable<vector<ShrinkableAny>>();
        shrinkVec.getMutableRef().reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec.getMutableRef().push_back(elemGen(rand));

        return shrinkListLike<list, T>(shrinkVec, minSize);
    }

    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<list<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<list<T>>::defaultMaxSize = 200;

}  // namespace proptest

#pragma once
#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/set.hpp"
#include "proptest/std/set.hpp"
#include "proptest/std/memory.hpp"

/**
 * @file set.hpp
 * @brief Arbitrary for set<T>
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for set<T> with configurable element generators and min/max sizes
 */
template <typename T>
class Arbi<set<T>> final : public ArbiContainer<set<T>> {
    using Set = set<T>;
    using ArbiContainer<Set>::minSize;
    using ArbiContainer<Set>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<set<T>>(_minSize, _maxSize), elemGen(Arbi<T>()) {}

    Arbi(GenFunction<T> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<set<T>>(_minSize, _maxSize), elemGen(_elemGen) {}

    Shrinkable<Set> operator()(Random& rand) const override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        Shrinkable<set<Shrinkable<T>>> shrinkableSet = make_shrinkable<set<Shrinkable<T>>>();

        while (shrinkableSet.getRef().size() < size) {
            auto elem = elemGen(rand);
            shrinkableSet.getMutableRef().insert(elem);
        }
        return shrinkSet(shrinkableSet, minSize);
    }

    shared_ptr<GeneratorBase<Set>> clone() const override {
        return util::make_shared<Arbi>(elemGen, minSize, maxSize);
    }
private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<set<T>>::defaultMaxSize = 200;

}  // namespace proptest

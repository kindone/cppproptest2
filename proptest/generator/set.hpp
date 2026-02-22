#pragma once
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/container_config.hpp"
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

    /**
     * @brief Constructor with named parameters (C++20 designated initializers)
     * @param config util::ContainerGenConfig<T> with optional .elemGen, .minSize, .maxSize
     */
    Arbi(const util::ContainerGenConfig<T>& config)
        : ArbiContainer<set<T>>(
              config.minSize.value_or(defaultMinSize),
              config.maxSize.value_or(defaultMaxSize)),
          elemGen(config.elemGen.value_or(Arbi<T>())) {}

    Shrinkable<Set> operator()(Random& rand) const override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkableSet = util::make_shared<set<Shrinkable<T>>>();

        while (shrinkableSet->size() < size) {
            auto elem = elemGen(rand);
            shrinkableSet->insert(elem);
        }
        return shrinkSet(shrinkableSet, minSize);
    }

private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<set<T>>::defaultMaxSize = 200;

}  // namespace proptest

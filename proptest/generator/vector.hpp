#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/generator/container_config.hpp"
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

    /**
     * @brief Constructor with named parameters (C++20 designated initializers)
     * @param config util::ContainerGenConfig<T> with optional .elemGen, .minSize, .maxSize
     */
    Arbi(const util::ContainerGenConfig<T>& config)
        : ArbiContainer<vector<T>>(
              config.minSize.value_or(defaultMinSize),
              config.maxSize.value_or(defaultMaxSize)),
          elemGen(config.elemGen.value_or(Arbi<T>())) {}

    Arbi setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Shrinkable<vector<T>> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkVec = make_shrinkable<vector<ShrinkableBase>>();
        shrinkVec.getMutableRef().reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec.getMutableRef().push_back(elemGen(rand));

        return shrinkListLike<vector, T>(shrinkVec, minSize);
    }

private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<vector<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<vector<T>>::defaultMaxSize = 200;

}  // namespace proptest

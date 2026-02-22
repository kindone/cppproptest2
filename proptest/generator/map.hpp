#pragma once
#include "proptest/Arbitrary.hpp"
#include "proptest/generator/container_config.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/map.hpp"
#include "proptest/generator/pair.hpp"
#include "proptest/std/map.hpp"
#include "proptest/std/set.hpp"
#include "proptest/std/memory.hpp"

/**
 * @file map.hpp
 * @brief Arbitrary for map<Key, T>
 */

namespace proptest {
/**
 * @ingroup Generators
 * @brief Arbitrary for map<Key, Value> with configurable pair generator and min/max sizes
 */
template <typename Key, typename Value>
class Arbi<map<Key, Value>> final : public ArbiContainer<map<Key, Value>> {
    using Pair = pair<Key, Value>;
    using Map = map<Key, Value>;
    using ArbiContainer<Map>::minSize;
    using ArbiContainer<Map>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<Map>(_minSize, _maxSize), pairGen(Arbi<Pair>()) {}

    Arbi(GenFunction<Pair> _pairGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize) : ArbiContainer<Map>(_minSize, _maxSize), pairGen(_pairGen) {}

    /**
     * @brief Constructor with named parameters (C++20 designated initializers)
     * @param config util::MapGenConfig<Key,Value> with optional .keyGen, .valueGen, .minSize, .maxSize
     */
    Arbi(const util::MapGenConfig<Key, Value>& config)
        : ArbiContainer<Map>(
              config.minSize.value_or(defaultMinSize),
              config.maxSize.value_or(defaultMaxSize)),
          pairGen(gen::pair(
              config.keyGen.value_or(Arbi<Key>()),
              config.valueGen.value_or(Arbi<Value>()))) {}

    Shrinkable<Map> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        set<Shrinkable<Pair>> pairShrSet;

        while (pairShrSet.size() < size) {
            auto pairShr = pairGen(rand);
            pairShrSet.insert(pairShr);
        }

        auto pairShrVecShr = Shrinkable<vector<ShrinkableBase>>(util::make_any<vector<ShrinkableBase>>(pairShrSet.begin(), pairShrSet.end()));

        return shrinkMap<Key, Value>(pairShrVecShr, minSize);
    }

    Arbi<Map> setPairGen(GenFunction<Pair> _pairGen)
    {
        pairGen = _pairGen;
        return *this;
    }

private:
    GenFunction<Pair> pairGen;
};

template <typename Key, typename Value>
size_t Arbi<map<Key, Value>>::defaultMinSize = 0;
template <typename Key, typename Value>
size_t Arbi<map<Key, Value>>::defaultMaxSize = 200;

}  // namespace proptest

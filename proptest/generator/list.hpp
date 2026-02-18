#pragma once

#include "proptest/Arbitrary.hpp"
#include "proptest/Random.hpp"
#include "proptest/shrinker/listlike.hpp"
#include "proptest/std/list.hpp"
#include "proptest/std/vector.hpp"
#include <concepts>

/**
 * @file list.hpp
 * @brief Arbitrary for list-like containers
 */
namespace proptest {

/**
 * @brief Concept for list-like containers that can be generated
 *
 * A list-like container must:
 * - Have value_type
 * - Support push_back operation
 */
template <typename C>
concept ListLikeContainer = requires(C& c, typename C::value_type v) {
    typename C::value_type;
    c.push_back(v);
};

/**
 * @ingroup Generators
 * @brief Arbitrary for list-like containers with push_back support (e.g., std::list) with configurable element generator and min/max sizes
 * 
 * Supports containers with one template parameter (e.g., custom containers) or two parameters with default allocator (e.g., std::list, std::vector)
 */
template <template <typename, typename...> class Container, typename T>
    requires ListLikeContainer<Container<T>>
class PROPTEST_API Arbi<Container<T>> final : public ArbiContainer<Container<T>> {
public:
    using ContainerType = Container<T>;
    using ArbiContainer<ContainerType>::minSize;
    using ArbiContainer<ContainerType>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi(size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize)
        : ArbiContainer<ContainerType>(_minSize, _maxSize), elemGen(Arbi<T>()) {}

    Arbi(GenFunction<T> _elemGen, size_t _minSize = defaultMinSize, size_t _maxSize = defaultMaxSize)
        : ArbiContainer<ContainerType>(_minSize, _maxSize), elemGen(_elemGen) {}

    Arbi setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Shrinkable<ContainerType> operator()(Random& rand) const override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        auto shrinkVec = make_shrinkable<vector<ShrinkableBase>>();
        shrinkVec.getMutableRef().reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec.getMutableRef().push_back(elemGen(rand));

        return shrinkListLike<Container, T>(shrinkVec, minSize);
    }

private:
    GenFunction<T> elemGen;
};

// Static member definitions for all list-like containers (including std::list with variadic template parameters)
template <template <typename, typename...> class Container, typename T>
    requires ListLikeContainer<Container<T>>
size_t Arbi<Container<T>>::defaultMinSize = 0;

template <template <typename, typename...> class Container, typename T>
    requires ListLikeContainer<Container<T>>
size_t Arbi<Container<T>>::defaultMaxSize = 200;

}  // namespace proptest

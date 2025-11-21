#pragma once

#include "proptest/api.hpp"
#include "proptest/Generator.hpp"
#include "proptest/util/define.hpp"

/**
 * @file Arbitrary.hpp
 * @brief Template definitions of Arbitrary<T> and its variants
 */

namespace proptest {

template <typename T>
class PROPTEST_API ArbitraryBase : public GeneratorBase<T>
{
    virtual shared_ptr<GeneratorBase<T>> clone() const override {
        return util::make_shared<Arbi<T>>(*dynamic_cast<Arbi<T>*>(const_cast<ArbitraryBase<T>*>(this)));
    }
};


/**
 * @ingroup Generators
 * @brief \ref Arbitrary (alias) or Arbi is the default generator for basic types.
 */
template <typename T>
struct PROPTEST_API Arbi : public ArbitraryBase<T>
{

    Shrinkable<T> operator()(Random&) const override {
        static_assert(is_same_v<T, void>, "Arbitrary for type T (proptest::Arbi<T>) must be defined before use");
    }
};

template <typename T>
struct ArbiContainer : public ArbitraryBase<T>
{
    ArbiContainer(size_t _minSize, size_t _maxSize) : minSize(_minSize), maxSize(_maxSize) {}

    Arbi<T>& setMinSize(size_t size)
    {
        minSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setMaxSize(size_t size)
    {
        maxSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setSize(size_t size)
    {
        minSize = size;
        maxSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setSize(size_t min, size_t max)
    {
        minSize = min;
        maxSize = max;
        return static_cast<Arbi<T>&>(*this);
    }

    size_t minSize;
    size_t maxSize;
};

/* Aliases */

/**
 * @ingroup Generators
 * @brief Arbitrary (alias for Arbi) is the default generator for basic types.
 */
template <typename... ARGS>
using Arbitrary = Arbi<ARGS...>;

template <typename... ARGS>
using ArbiBase = ArbitraryBase<ARGS...>;

template <typename... ARGS>
using ArbitraryContainer = ArbiContainer<ARGS...>;

#define DEFINE_ARBITRARY(TYPE, ...)                                                                         \
    template <>                                                                                             \
    struct Arbi<TYPE> : ArbiBase<TYPE>                                                                      \
    {                                                                                                       \
        ::proptest::Shrinkable<TYPE> operator()(::proptest::Random& rand) const { return (__VA_ARGS__)()(rand); } \
    }

} // namespace proptest


#ifdef PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

#define EXTERN_DECLARE_ARBITRARY(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::Arbi, TYPE)
//DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_DECLARE_ARBITRARY);

#endif // PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

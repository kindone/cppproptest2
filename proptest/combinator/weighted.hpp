#pragma once

#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"

/**
 * @file weighted.hpp
 * @brief All weighted-related types for elementOf and oneOf
 */

namespace proptest {

namespace util {

template <typename T>
struct WeightedValue;

struct WeightedValueBase
{
    template <typename T>
    WeightedValueBase(const WeightedValue<T>& weighted) : value(weighted.value), weight(weighted.weight) {}
    WeightedValueBase(const Any& _value, double _weight) : value(_value), weight(_weight) {}

    Any value;
    double weight;
};

template <typename T>
struct WeightedValue : WeightedValueBase
{
    WeightedValue(const WeightedValueBase& base) : WeightedValueBase(base) {}
    WeightedValue(const Any& _value, double _weight) : WeightedValueBase(_value, _weight) {}
};

template <typename T>
struct Weighted;

// Generator form: (Random& -> Shrinkable<T>) + weight
struct WeightedBase
{
    WeightedBase(const Function1<ShrinkableBase>& _func, double _weight) : func(_func), weight(_weight) {}
    template <typename T>
    WeightedBase(const Weighted<T>& weighted) : func(weighted.func), weight(weighted.weight) {}

    Function1<ShrinkableBase> func;
    double weight;
};

template <typename T>
struct Weighted : WeightedBase
{
    using type = T;

    Weighted(const WeightedBase& base) : WeightedBase(base) {}
    Weighted(const Function1<ShrinkableBase>& _func, double _weight) : WeightedBase(_func, _weight) {}
};

}  // namespace util

}  // namespace proptest

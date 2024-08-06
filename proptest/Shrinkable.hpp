#pragma once

#include <memory>
#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/Stream.hpp"
#include "proptest/util/define.hpp"

/**
 * @file Shrinkable.hpp
 * @brief Shrinkable class that represents a value that can be shrunk
 */

namespace proptest {

struct ShrinkableBase;
template <typename T> struct Shrinkable;

template <typename T, typename... Args> Shrinkable<T> make_shrinkable(Args&&... args);

struct PROPTEST_API ShrinkableBase
{
    using StreamElementType = ShrinkableBase;
    using StreamType = ::proptest::Stream;

    explicit ShrinkableBase(const Any& _value);

    template <typename T>
    ShrinkableBase(const Shrinkable<T>& other) : value(other.value), shrinksGen(other.shrinksGen) {}

    ShrinkableBase clear() const;

    ShrinkableBase with(const StreamType& otherShrinks) const;

    ShrinkableBase with(Function<StreamType()> otherStream) const;

    // template <typename T> T get() const { return value.getRef<T>(); }
    template <typename T> const T& getRef() const { return value.getRef<T>(); }
    template <typename T> T& getMutableRef() { return value.getMutableRef<T>(); }
    Any getAny() const;

    ShrinkableBase clone() const;

    StreamType getShrinks() const;

    ShrinkableBase map(Function1<Any> transformer) const;

    ShrinkableBase flatMap(Function1<ShrinkableBase> transformer) const;

    template <typename U, typename T>
    ShrinkableBase mapShrinkable(Function<ShrinkableBase(ShrinkableBase&)> transformer) const;

    // provide filtered generation, shrinking
    ShrinkableBase filter(Function1<bool> criteria) const;

    // provide filtered generation, shrinking
    ShrinkableBase filter(Function1<bool> criteria, int tolerance) const;

    // concat: continues with then after horizontal dead end
    ShrinkableBase concatStatic(const StreamType& then) const;

    // concat: extend shrinks stream with function taking parent as argument
    ShrinkableBase concat(Function<StreamType(const ShrinkableBase&)> then) const;

    // andThen: continues with then after vertical dead end
    ShrinkableBase andThenStatic(const StreamType& then) const;

    ShrinkableBase andThen(Function<StreamType(const ShrinkableBase&)> then) const;

    ShrinkableBase take(int n) const;

private:
    ShrinkableBase(const Any& _value, const Function<StreamType()>& _shrinksGen);

    Any value;
    Function<StreamType()> shrinksGen;

public:

    template <typename T>
    friend struct Shrinkable;

    template <typename T, typename... ARGS>
    friend Shrinkable<T> proptest::make_shrinkable(ARGS&&... args);
};

template <typename T>
struct Shrinkable : public ShrinkableBase
{
    using type = T;
    using TStreamType = ::proptest::Stream;

    Shrinkable(const ShrinkableBase& base) : ShrinkableBase(base) {
        static_assert(sizeof(ShrinkableBase) == sizeof(Shrinkable<T>), "Shrinkable<T> must have same size as ShrinkableBase");
    }

    template <typename U>
    Shrinkable(const Shrinkable<U>& otherShr) : ShrinkableBase(otherShr) {
    }

    template <same_as<T> TT=T>
        requires (!is_same_v<TT, Any>)
    Shrinkable(const T& _value) : ShrinkableBase(Any(_value)) {}

    explicit Shrinkable(Any _value) : ShrinkableBase(_value) {}

    Shrinkable(const Any& _value, const Function<StreamType()>& _shrinksGen) : ShrinkableBase(_value, _shrinksGen) {}

    Shrinkable with(const StreamType& otherShrinks) const {
        return ShrinkableBase::with(otherShrinks);
    }

    Shrinkable with(Function<StreamType()> otherStream) const {
        return ShrinkableBase::with(otherStream);
    }

    // T get() const { return ShrinkableBase::get<T>(); }
    const T& getRef() const { return ShrinkableBase::getRef<T>(); }
    T& getMutableRef() { return ShrinkableBase::getMutableRef<T>(); }

    Shrinkable clone() const { return ShrinkableBase::clone(); }

    template <typename U>
    Shrinkable<U> map(Function1<Any> transformer) const {
        return ShrinkableBase::map(transformer);
    }

    template <typename U>
    Shrinkable<U> flatMap(Function1<ShrinkableBase> transformer) const {
        return ShrinkableBase::flatMap(transformer);
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(Function<ShrinkableBase(ShrinkableBase&)> transformer) const {
        return ShrinkableBase::mapShrinkable<U, T>(transformer);
    }

    // provide filtered generation, shrinking
    Shrinkable filter(Function1<bool> criteria) const {
        return ShrinkableBase::filter(criteria);
    }

    // provide filtered generation, shrinking
    Shrinkable filter(Function1<bool> criteria, int tolerance) const {
        return ShrinkableBase::filter(criteria, tolerance);
    }

    // concat: continues with then after horizontal dead end
    Shrinkable concatStatic(const StreamType& then) const { return ShrinkableBase::concatStatic(then); }
    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable concat(Function<StreamType(ShrinkableBase&)> then) const { return ShrinkableBase::concat(then); }
    // andThen: continues with then after vertical dead end
    Shrinkable andThenStatic(const StreamType& then) const { return ShrinkableBase::andThenStatic(then); }

    Shrinkable andThen(Function<StreamType(ShrinkableBase&)> then) const { return ShrinkableBase::andThen(then); }

    Shrinkable take(int n) const { return ShrinkableBase::take(n); }
};

template <typename T, typename... ARGS>
Shrinkable<T> make_shrinkable(ARGS&&... args)
{
    return Shrinkable<T>{util::make_any<T>(util::forward<ARGS>(args)...)};
}


} // namespace proptest

#ifdef PROPTEST_ENABLE_EXPLICIT_INSTANTIATION

#define EXTERN_DECLARE_SHRINKABLE(TYPE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::Shrinkable, TYPE)
#define EXTERN_DECLARE_FUNCTION(SIGNATURE) EXTERN_DECLARE_STRUCT_TYPE(::proptest::Function, SIGNATURE)
#define EXTERN_GENERATOR_FUNCTION(TYPE) EXTERN_DECLARE_FUNCTION(::proptest::Shrinkable<TYPE>(::proptest::Random&))

DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_DECLARE_SHRINKABLE);
EXTERN_DECLARE_SHRINKABLE(::proptest::vector<::proptest::ShrinkableBase>);

DEFINE_FOR_ALL_BASIC_TYPES(EXTERN_GENERATOR_FUNCTION);
EXTERN_GENERATOR_FUNCTION(::proptest::vector<::proptest::ShrinkableBase>);
EXTERN_DECLARE_FUNCTION(::proptest::ShrinkableBase(::proptest::Random&));

#endif // PROPTEST_ENABLE_EXPLICIT_INSTANTIATION


// compare function
namespace std {

template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};

template <typename T, typename U>
class less<proptest::Shrinkable<pair<T, U>>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<pair<T,U>>& lhs, const proptest::Shrinkable<pair<T,U>>& rhs) const
    {
        return lhs.getRef().first < rhs.getRef().first;
    }
};

}  // namespace std

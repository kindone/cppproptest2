#pragma once

#include <memory>
#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/util/lazy.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/Stream.hpp"

namespace proptest {

struct ShrinkableBase;
template <typename T> struct Shrinkable;

template <typename T, typename... Args> Shrinkable<T> make_shrinkable(Args&&... args);

struct PROPTEST_API ShrinkableBase
{
    using StreamElementType = ShrinkableBase;
    using StreamType = ::proptest::Stream;

    explicit ShrinkableBase(Any _value);

    template <typename T>
    ShrinkableBase(const Shrinkable<T>& other) : value(other.value), shrinks(other.shrinks) {}

    ShrinkableBase clear() const;

    ShrinkableBase with(const StreamType& otherShrinks) const;

    ShrinkableBase with(Function<StreamType()> otherStream) const;

    template <typename T> T get() const { return value.getRef<T>(); }
    template <typename T> const T& getRef() const { return value.getRef<T>(); }
    template <typename T> T& getMutableRef() { return value.getMutableRef<T>(); }
    Any getAny() const;

    ShrinkableBase clone() const;

    StreamType getShrinks() const;

    ShrinkableBase map(Function1 transformer) const;

    ShrinkableBase flatMap(Function1 transformer) const;

    template <typename U, typename T>
    ShrinkableBase mapShrinkable(Function<ShrinkableBase(ShrinkableBase&)> transformer) const;

    // provide filtered generation, shrinking
    ShrinkableBase filter(Function1 criteria) const;

    // provide filtered generation, shrinking
    ShrinkableBase filter(Function1 criteria, int tolerance) const;

    // concat: continues with then after horizontal dead end
    ShrinkableBase concatStatic(const StreamType& then) const;

    // concat: extend shrinks stream with function taking parent as argument
    ShrinkableBase concat(Function<StreamType(const ShrinkableBase&)> then) const;

    // andThen: continues with then after vertical dead end
    ShrinkableBase andThenStatic(const StreamType& then) const;

    ShrinkableBase andThen(Function<StreamType(const ShrinkableBase&)> then) const;

    ShrinkableBase take(int n) const;

private:
    ShrinkableBase(const Any& _value, const Lazy<StreamType>& _shrinks);

    Any value;
    Lazy<StreamType> shrinks;

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

    Shrinkable(const ShrinkableBase& base) : ShrinkableBase(base) {}

    template <typename U>
        requires (is_same_v<T, Any>)
    Shrinkable(const Shrinkable<U>& otherShr) : ShrinkableBase(otherShr) {}

    template <same_as<T> TT=T>
        requires (!is_same_v<TT, Any>)
    Shrinkable(const T& _value) : ShrinkableBase(Any(_value)) {}

    explicit Shrinkable(Any _value) : ShrinkableBase(_value) {}

    Shrinkable with(const StreamType& otherShrinks) const {
        return ShrinkableBase::with(otherShrinks);
    }

    Shrinkable with(Function<StreamType()> otherStream) const {
        return ShrinkableBase::with(otherStream);
    }

    T get() const { return ShrinkableBase::get<T>(); }
    const T& getRef() const { return ShrinkableBase::getRef<T>(); }
    T& getMutableRef() { return ShrinkableBase::getMutableRef<T>(); }

    Shrinkable clone() const { return ShrinkableBase::clone(); }

    template <typename U>
    Shrinkable<U> map(Func1<U(T&)> transformer) const {
        return ShrinkableBase::map(transformer);
    }

    template <typename U>
    Shrinkable<U> flatMap(Func1<ShrinkableBase(T&)> transformer) const {
        return ShrinkableBase::flatMap(transformer);
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(Function<ShrinkableBase(ShrinkableBase&)> transformer) const {
        return ShrinkableBase::mapShrinkable<U, T>(transformer);
    }

    // provide filtered generation, shrinking
    Shrinkable filter(Func1<bool(T&)> criteria) const {
        return ShrinkableBase::filter(criteria);
    }

    // provide filtered generation, shrinking
    Shrinkable filter(Func1<bool(T&)> criteria, int tolerance) const {
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

using ShrinkableAny = Shrinkable<Any>;

template <typename T, typename... ARGS>
Shrinkable<T> make_shrinkable(ARGS&&... args)
{
    return Shrinkable<T>{util::make_any<T>(util::forward<ARGS>(args)...)};
}

} // namespace proptest



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

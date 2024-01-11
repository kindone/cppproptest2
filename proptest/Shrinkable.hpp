#pragma once

#include <memory>
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/util/lazy.hpp"
#include "proptest/TypedStream.hpp"

namespace proptest {

template <typename T> struct Shrinkable;

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable
{
    using type = T;
    using Stream = TypedStream<Shrinkable<T>>;

    Shrinkable(Any _value) : value(_value), shrinks(Stream::empty()) {}

    Shrinkable& operator=(const Shrinkable& other) {
        value = other.value;
        shrinks = other.shrinks;
        return *this;
    }

    Shrinkable with(const Stream& otherShrinks) const {
        return Shrinkable(value, otherShrinks);
    }

    Shrinkable with(Function<Stream()> otherStream) const {
        return Shrinkable(value, Lazy<Stream>(otherStream));
    }

    // operator T() const { return get(); }
    T get() const { return value.getRef<T>(); }
    const T& getRef() const { return value.getRef<T>(); }
    Any getAny() const { return value; }

    Stream getShrinks() { return *shrinks; }

    template <typename U>
    Shrinkable<U> map(Function<U(const T&)> transformer) const {
        return Shrinkable<U>{transformer(value.getRef<T>()), shrinks->template transform<Shrinkable<U>>([transformer](const Shrinkable<T>& shr) -> Shrinkable<U>{
            return shr.map<U>(transformer);
        })};
    }

    template <typename U>
    Shrinkable<U> flatMap(Function<Shrinkable<U>(const T&)> transformer) const {
        return transformer(value.getRef<T>()).with(shrinks->template transform<Shrinkable<U>>([transformer](const Shrinkable<T>& shr) -> Shrinkable<U>{
            return shr.flatMap<U>(transformer);
        }));
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(Function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const;

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria) const {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument("cannot apply criteria");
        else
            return with(shrinks->filter([criteria](const Shrinkable<T>& shr) -> bool {
                return criteria(shr.getRef());
            }));
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria, int tolerance) {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument("cannot apply criteria");
        else
            return with(shrinks->take(tolerance).filter([criteria, tolerance](const Shrinkable<T>& shr) -> bool {
                return criteria(shr.getRef());
            }, tolerance));
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(const Stream& then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.concatStatic(then);
        });
        return with(shrinksWithThen.concat(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(Function<Stream(const Shrinkable<T>&)> then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.concat(then(shr));
        });
        return with(shrinksWithThen.concat(then(*this)));
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(const Stream& then) const {
        if(shrinks.isEmpty())
            return with(then);
        else
            return with(shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThenStatic(then);
            }));
    }

    Shrinkable<T> andThen(Function<Stream(const Shrinkable<T>&)> then) const {
        if(shrinks.isEmpty())
            return with(then(*this));
        else
            return with(shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThen(then(shr));
            }));
    }

    Shrinkable<T> take(int n) const {
        return with(shrinks->template transform<Shrinkable<T>>([n](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.take(n);
        }));
    }

private:
    Shrinkable(Any _value, const Lazy<Stream>& _shrinks) : value(_value), shrinks(_shrinks) {}

    Any value;
    Lazy<Stream> shrinks;

public:

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args)
{
    return Shrinkable<T>{util::make_any<T>(util::forward<Args>(args)...)};
}


} // namespace proptest
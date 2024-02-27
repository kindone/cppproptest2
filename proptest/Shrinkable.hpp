#pragma once

#include <memory>
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/util/lazy.hpp"
#include "proptest/Stream.hpp"

namespace proptest {

template <typename T> struct Shrinkable;

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable
{
    using type = T;
    using StreamType = Stream<Shrinkable<T>>;

    // for Shrinkable<Any> a.k.a. ShrinkableAny
    template <typename U>
        requires (is_same_v<T, Any>)
    Shrinkable(const Shrinkable<U>& otherShr) : Shrinkable(otherShr.template map<Any>([](const U& u) -> Any {
        return Any(u);
    })) {}

    template <typename U=T>
        requires (!is_same_v<U, Any>)
    Shrinkable(const T& _value) : Shrinkable(Any(_value)) {}

    explicit Shrinkable(Any _value) : value(_value), shrinks(StreamType::empty()) {}

    Shrinkable clear() const {
        return Shrinkable{value};
    }

    Shrinkable with(const StreamType& otherShrinks) const {
        if constexpr(is_same_v<T, Any>) {
            if(!otherShrinks.isEmpty() && otherShrinks.getHeadRef().getRef().type() != value.type())
                throw invalid_argument("cannot apply stream to shrinkable: " + string(otherShrinks.getHeadRef().getRef().type().name()) + " to " + string(value.type().name()));
        }
        return Shrinkable(value, otherShrinks);
    }

    Shrinkable with(Function<StreamType()> otherStream) const {
        if constexpr(is_same_v<T, Any>) {
            auto otherShrinks = otherStream();
            if(!otherShrinks.isEmpty() && otherShrinks.getHeadRef().getRef().type() != value.type())
                throw invalid_argument("cannot apply stream to shrinkable: " + string(otherShrinks.getHeadRef().getRef().type().name()) + " to " + string(value.type().name()));
        }
        return Shrinkable(value, Lazy<StreamType>(otherStream));
    }

    // operator T() const { return get(); }
    T get() const { return value.getRef<T>(); }
    const T& getRef() const { return value.getRef<T>(); }
    T& getMutableRef() { return value.getMutableRef<T>(); }
    Any getAny() const { return value; }

    Shrinkable clone() const {
        return Shrinkable(value.clone(), shrinks);
    }

    StreamType getShrinks() const { return *shrinks; }

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
    Shrinkable<U> mapShrinkable(Function<Shrinkable<U>(const Shrinkable&)> transformer) const;

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria) const {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument("cannot apply criteria");
        else
            return with(shrinks->filter([criteria](const Shrinkable& shr) -> bool {
                return criteria(shr.getRef());
            }).template transform<Shrinkable<T>>([criteria](const Shrinkable& shr) {
                return shr.filter(criteria);
            }));
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria, int tolerance) const {

        static Function<StreamType(const StreamType&,Function<bool(const T&)>, int)> filterStream = +[](const StreamType& stream, Function<bool(const T&)> _criteria, int _tolerance) {
            if(stream.isEmpty())
                return StreamType::empty();
            else {
                for(auto itr = stream.iterator(); itr.hasNext();) {
                    auto shr = itr.next();
                    auto tail = itr.stream;
                    if(_criteria(shr.getRef())) {
                        return StreamType{shr, [tail, _criteria, _tolerance]() { return filterStream(tail, _criteria, _tolerance);}};
                    }
                    // extract from shr's children
                    else {
                        return filterStream(shr.getShrinks().take(_tolerance).concat(tail), _criteria, _tolerance);
                    }
                }
                return StreamType::empty();
            }
        };
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument("cannot apply criteria");

        return with(filterStream(getShrinks(), criteria, tolerance).template transform<Shrinkable>([criteria, tolerance](const Shrinkable& shr) {
            return shr.filter(criteria, tolerance);
        }));
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(const StreamType& then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.concatStatic(then);
        });
        return with(shrinksWithThen.concat(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(Function<StreamType(const Shrinkable<T>&)> then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.concat(then);
        });
        return with(shrinksWithThen.concat(then(*this)));
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(const StreamType& then) const {
        if(shrinks->isEmpty())
            return with(then);
        else
            return with(shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThenStatic(then);
            }));
    }

    Shrinkable<T> andThen(Function<StreamType(const Shrinkable<T>&)> then) const {
        if(shrinks->isEmpty())
            return with(then(*this));
        else
            return with(shrinks->template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThen(then);
            }));
    }

    Shrinkable<T> take(int n) const {
        return with(shrinks->template transform<Shrinkable<T>>([n](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.take(n);
        }));
    }

private:
    Shrinkable(Any _value, const Lazy<StreamType>& _shrinks) : value(_value), shrinks(_shrinks) {
        if constexpr(is_same_v<T, Any>) {
            if(!_shrinks->isEmpty() && _shrinks->getHeadRef().getRef().type() != value.type())
                throw invalid_argument("cannot apply stream to shrinkable: " + string(_shrinks->getHeadRef().getRef().type().name()) + " to " + string(value.type().name()));
        }
    }

    Any value;
    Lazy<StreamType> shrinks;

public:

    template <typename U, typename... ARGS>
    friend Shrinkable<U> make_shrinkable(ARGS&&... args);

    template <typename U>
    friend struct Shrinkable;
};

template <typename T, typename... ARGS>
Shrinkable<T> make_shrinkable(ARGS&&... args)
{
    return Shrinkable<T>{util::make_any<T>(util::forward<ARGS>(args)...)};
}

using ShrinkableAny = Shrinkable<Any>;

// explicit instantiation of Shrinkable<Any>
extern template struct Shrinkable<Any>;

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

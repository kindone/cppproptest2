#pragma once

#include <memory>
#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/util/lazy.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/Stream.hpp"

#define PROPTEST_UNTYPED_SHRINKABLE 1


namespace proptest {

#ifndef PROPTEST_UNTYPED_SHRINKABLE

namespace typed {
template <typename T> requires (!std::is_const_v<T>) struct Shrinkable;
}

template <typename T, typename... Args> typed::Shrinkable<T> make_shrinkable(Args&&... args);

#else

namespace untyped {
struct Shrinkable;
}

template <typename T, typename... Args> untyped::Shrinkable make_shrinkable(Args&&... args);

#endif // PROPTEST_UNTYPED_SHRINKABLE


namespace untyped {

struct PROPTEST_API Shrinkable
{
    using StreamType = ::proptest::Stream<Shrinkable>;

    // for Shrinkable<Any> a.k.a. ShrinkableAny

    template <typename U, typename T>
        requires (!is_same_v<U, Any>)
    Shrinkable(const T& _value) : Shrinkable(Any(_value)) {}

    explicit Shrinkable(Any _value) : value(_value), shrinks(StreamType::empty()) {}

    Shrinkable clear() const {
        return Shrinkable{value};
    }

    Shrinkable with(const StreamType& otherShrinks) const {
        return Shrinkable(value, otherShrinks);
    }

    Shrinkable with(Function<StreamType()> otherStream) const {
        return Shrinkable(value, Lazy<StreamType>(otherStream));
    }


    template <typename T> T get() const { return value.getRef<T>(); }
    template <typename T> const T& getRef() const { return value.getRef<T>(); }
    template <typename T> T& getMutableRef() { return value.getMutableRef<T>(); }
    Any getAny() const { return value; }

    Shrinkable clone() const {
        return Shrinkable(value.clone(), shrinks);
    }

    StreamType getShrinks() const { return *shrinks; }

    template <typename U, typename T>
    Shrinkable map(Function<U(const T&)> transformer) const {
        return Shrinkable(transformer(value.getRef<T>())).with([shrinks = this->shrinks, transformer]() -> ::proptest::Stream<Shrinkable> {
            return shrinks->template transform<Shrinkable,Shrinkable>([transformer](const Shrinkable& shr) -> Shrinkable {
                    return shr.map<U, T>(transformer);
                });
            }
        );
    }

    template <typename U, typename T>
    Shrinkable flatMap(Function<Shrinkable(const T&)> transformer) const {
        return transformer(value.getRef<T>()).with([shrinks = this->shrinks, transformer]() { return shrinks->template transform<Shrinkable,Shrinkable>([transformer](const Shrinkable& shr) -> Shrinkable {
                    return shr.flatMap<U, T>(transformer);
                });
            }
        );
    }

    template <typename U, typename T>
    Shrinkable mapShrinkable(Function<Shrinkable(const Shrinkable&)> transformer) const;

    // provide filtered generation, shrinking
    template <typename T>
    Shrinkable filter(Function<bool(const T&)> criteria) const {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");
        else
            return with(shrinks->template filter<Shrinkable>([criteria](const Shrinkable& shr) -> bool {
                return criteria(shr.getRef<T>());
            }).template transform<Shrinkable,Shrinkable>([criteria](const Shrinkable& shr) {
                return shr.filter<T>(criteria);
            }));
    }

    // provide filtered generation, shrinking
    template <typename T>
    Shrinkable filter(Function<bool(const T&)> criteria, int tolerance) const {

        static Function<StreamType(const StreamType&,Function<bool(const T&)>, int)> filterStream = +[](const StreamType& stream, Function<bool(const T&)> _criteria, int _tolerance) {
            if(stream.isEmpty())
                return StreamType::empty();
            else {
                for(auto itr = stream.template iterator<Shrinkable>(); itr.hasNext();) {
                    auto shr = itr.next();
                    auto tail = itr.stream;
                    if(_criteria(shr.getRef<T>())) {
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
            throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");

        return with(filterStream(getShrinks(), criteria, tolerance).template transform<Shrinkable,Shrinkable>([criteria, tolerance](const Shrinkable& shr) {
            return shr.filter<T>(criteria, tolerance);
        }));
    }

    // concat: continues with then after horizontal dead end
    Shrinkable concatStatic(const StreamType& then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable,Shrinkable>([then](const Shrinkable& shr) -> Shrinkable {
            return shr.concatStatic(then);
        });
        return with(shrinksWithThen.concat(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable concat(Function<StreamType(const Shrinkable&)> then) const {
        return with([copy = *this, shrinks = this->shrinks, then]() {
            auto shrinksWithThen = shrinks->template transform<Shrinkable,Shrinkable>([then](const Shrinkable& shr) -> Shrinkable {
                return shr.concat(then);
            });
            return shrinksWithThen.concat([=]() { return then(copy); });
        });
    }

    // andThen: continues with then after vertical dead end
    Shrinkable andThenStatic(const StreamType& then) const {
        if(shrinks->isEmpty())
            return with(then);
        else
            return with(shrinks->template transform<Shrinkable,Shrinkable>([then](const Shrinkable& shr) -> Shrinkable {
                return shr.andThenStatic(then);
            }));
    }

    Shrinkable andThen(Function<StreamType(const Shrinkable&)> then) const {
        if(shrinks->isEmpty())
            return with(then(*this));
        else
            return with(shrinks->template transform<Shrinkable,Shrinkable>([then](const Shrinkable& shr) -> Shrinkable {
                return shr.andThen(then);
            }));
    }

    Shrinkable take(int n) const {
        return with(shrinks->template transform<Shrinkable,Shrinkable>([n](const Shrinkable& shr) -> Shrinkable {
            return shr.take(n);
        }));
    }

private:
    Shrinkable(const Any& _value, const Lazy<StreamType>& _shrinks) : value(_value), shrinks(_shrinks) {
    }

    Any value;
    Lazy<StreamType> shrinks;

public:

#ifdef PROPTEST_UNTYPED_SHRINKABLE
    template <typename T, typename... ARGS>
    friend Shrinkable proptest::make_shrinkable(ARGS&&... args);
#endif
};

} // namespace untyped


namespace typed {

template <typename T>
    requires (!std::is_const_v<T>)
struct PROPTEST_API Shrinkable
{
    using type = T;
    using StreamType = ::proptest::Stream<Shrinkable<T>>;

    // for Shrinkable<Any> a.k.a. ShrinkableAny
    template <typename U>
        requires (is_same_v<T, Any>)
    Shrinkable(const Shrinkable<U>& otherShr) : Shrinkable(otherShr.template map<Any>(+[](const U& u) -> Any {
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
        return Shrinkable(value, otherShrinks);
    }

    Shrinkable with(Function<StreamType()> otherStream) const {
        return Shrinkable(value, Lazy<StreamType>(otherStream));
    }

    template <same_as<T> TT = T>
    T get() const { return value.getRef<T>(); }

    template <same_as<T> TT = T>
    const T& getRef() const { return value.getRef<T>(); }

    template <same_as<T> TT = T>
    T& getMutableRef() { return value.getMutableRef<T>(); }

    Any getAny() const { return value; }

    Shrinkable clone() const {
        return Shrinkable(value.clone(), shrinks);
    }

    StreamType getShrinks() const { return *shrinks; }

    template <typename U, same_as<T> TT = T>
    Shrinkable<U> map(Function<U(const T&)> transformer) const {
        return Shrinkable<U>(transformer(value.getRef<T>())).with([shrinks = this->shrinks, transformer]() -> ::proptest::Stream<Shrinkable<U>> {
            return shrinks->template transform<Shrinkable<U>,Shrinkable<T>>([transformer](const Shrinkable<T>& shr) -> Shrinkable<U> {
                    return shr.map<U>(transformer);
                });
            }
        );
    }

    template <typename U, same_as<T> TT = T>
    Shrinkable<U> flatMap(Function<Shrinkable<U>(const T&)> transformer) const {
        return transformer(value.getRef<T>()).with([shrinks = this->shrinks, transformer]() { return shrinks->template transform<Shrinkable<U>,Shrinkable<T>>([transformer](const Shrinkable<T>& shr) -> Shrinkable<U>{
                    return shr.flatMap<U>(transformer);
                });
            }
        );
    }

    template <typename U, same_as<T> TT = T>
    Shrinkable<U> mapShrinkable(Function<Shrinkable<U>(const Shrinkable&)> transformer) const;

    // provide filtered generation, shrinking
    template <same_as<T> TT = T>
    Shrinkable<T> filter(Function<bool(const T&)> criteria) const {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");
        else
            return with(shrinks->template filter<Shrinkable<T>>([criteria](const Shrinkable& shr) -> bool {
                return criteria(shr.getRef());
            }).template transform<Shrinkable<T>,Shrinkable<T>>([criteria](const Shrinkable& shr) {
                return shr.filter(criteria);
            }));
    }

    // provide filtered generation, shrinking
    template <same_as<T> TT = T>
    Shrinkable<T> filter(Function<bool(const T&)> criteria, int tolerance) const {

        static Function<StreamType(const StreamType&,Function<bool(const T&)>, int)> filterStream = +[](const StreamType& stream, Function<bool(const T&)> _criteria, int _tolerance) {
            if(stream.isEmpty())
                return StreamType::empty();
            else {
                for(auto itr = stream.template iterator<Shrinkable<T>>(); itr.hasNext();) {
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
            throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");

        return with(filterStream(getShrinks(), criteria, tolerance).template transform<Shrinkable,Shrinkable>([criteria, tolerance](const Shrinkable& shr) {
            return shr.filter(criteria, tolerance);
        }));
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(const StreamType& then) const {
        auto shrinksWithThen = shrinks->template transform<Shrinkable<T>,Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.concatStatic(then);
        });
        return with(shrinksWithThen.concat(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(Function<StreamType(const Shrinkable<T>&)> then) const {
        return with([copy = *this, shrinks = this->shrinks, then]() {
            auto shrinksWithThen = shrinks->template transform<Shrinkable<T>,Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.concat(then);
            });
            return shrinksWithThen.concat([=]() { return then(copy); });
        });
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(const StreamType& then) const {
        if(shrinks->isEmpty())
            return with(then);
        else
            return with(shrinks->template transform<Shrinkable<T>,Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThenStatic(then);
            }));
    }

    Shrinkable<T> andThen(Function<StreamType(const Shrinkable<T>&)> then) const {
        if(shrinks->isEmpty())
            return with(then(*this));
        else
            return with(shrinks->template transform<Shrinkable<T>,Shrinkable<T>>([then](const Shrinkable<T>& shr) -> Shrinkable<T> {
                return shr.andThen(then);
            }));
    }

    Shrinkable<T> take(int n) const {
        return with(shrinks->template transform<Shrinkable<T>,Shrinkable<T>>([n](const Shrinkable<T>& shr) -> Shrinkable<T> {
            return shr.take(n);
        }));
    }

private:
    Shrinkable(const Any& _value, const Lazy<StreamType>& _shrinks) : value(_value), shrinks(_shrinks) {
    }

    Any value;
    Lazy<StreamType> shrinks;

public:

    template <typename U, typename... ARGS>
    friend Shrinkable<U> proptest::make_shrinkable(ARGS&&... args);

    template <typename U>
        requires (!std::is_const_v<U>)
    friend struct Shrinkable;
};

} // namespace typed



#ifndef PROPTEST_UNTYPED_SHRINKABLE

template <typename T> using Shrinkable = typed::Shrinkable<T>;

// explicit instantiation of Shrinkable<Any>
namespace typed {
extern template struct PROPTEST_API Shrinkable<Any>;
}

#else

template <typename T> using Shrinkable = untyped::Shrinkable;

#endif // PROPTEST_UNTYPED_SHRINKABLE

using ShrinkableAny = Shrinkable<Any>;

template <typename T, typename... ARGS>
Shrinkable<T> make_shrinkable(ARGS&&... args)
{
    return Shrinkable<T>{util::make_any<T>(util::forward<ARGS>(args)...)};
}

#ifndef PROPTEST_UNTYPED_STREAM
namespace typed {
extern template struct PROPTEST_API Stream<Shrinkable<vector<Shrinkable<Any>>>>;
}
#endif // PROPTEST_UNTYPED_STREAM

} // namespace proptest



// compare function
namespace std {

#ifndef PROPTEST_UNTYPED_SHRINKABLE

template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.getRef<T>() < rhs.getRef<T>();
    }
};

template <typename T, typename U>
class less<proptest::Shrinkable<pair<T, U>>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<pair<T,U>>& lhs, const proptest::Shrinkable<pair<T,U>>& rhs) const
    {
        return lhs.getRef<pair<T,U>>().first < rhs.getRef<pair<T,U>>().first;
    }
};

#else

// class less<proptest::untyped::Shrinkable> {
// public:
//     constexpr bool operator()(const proptest::untyped::Shrinkable& lhs, const proptest::untyped::Shrinkable& rhs) const
//     {
//         return lhs.getRef<T>() < rhs.getRef<T>();
//     }
// };

#endif // PROPTEST_UNTYPED_SHRINKABLE

}  // namespace std

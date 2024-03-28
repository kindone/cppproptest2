#pragma once

#include <memory>
#include "proptest/api.hpp"
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/util/lazy.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/Stream.hpp"

//#define PROPTEST_UNTYPED_SHRINKABLE 1


namespace proptest {

namespace typed {
    template <typename T> requires (!std::is_const_v<T>) struct Shrinkable;
}

namespace untyped {
    struct ShrinkableBase;
    template <typename T> struct Shrinkable;
}


#ifndef PROPTEST_UNTYPED_SHRINKABLE
template <typename T, typename... Args> typed::Shrinkable<T> make_shrinkable(Args&&... args);
#else
template <typename T, typename... Args> untyped::Shrinkable<T> make_shrinkable(Args&&... args);
#endif // PROPTEST_UNTYPED_SHRINKABLE

namespace untyped {

struct PROPTEST_API ShrinkableBase
{
    using StreamType = ::proptest::Stream<ShrinkableBase>;

    explicit ShrinkableBase(Any _value) : value(_value), shrinks(StreamType::empty()) {}

    template <typename T>
    ShrinkableBase(const Shrinkable<T>& other) : value(other.value), shrinks(other.shrinks) {}

    ShrinkableBase clear() const {
        return ShrinkableBase{value};
    }

    ShrinkableBase with(const StreamType& otherShrinks) const {
        return ShrinkableBase(value, otherShrinks);
    }

    ShrinkableBase with(Function<StreamType()> otherStream) const {
        return ShrinkableBase(value, Lazy<StreamType>(otherStream));
    }


    template <typename T> T get() const { return value.getRef<T>(); }
    template <typename T> const T& getRef() const { return value.getRef<T>(); }
    template <typename T> T& getMutableRef() { return value.getMutableRef<T>(); }
    Any getAny() const { return value; }

    ShrinkableBase clone() const {
        return ShrinkableBase(value.clone(), shrinks);
    }

    StreamType getShrinks() const { return *shrinks; }

    template <typename U, typename T>
    ShrinkableBase map(Function<U(const T&)> transformer) const {
        return ShrinkableBase(transformer(value.getRef<T>())).with([shrinks = this->shrinks, transformer]() -> ::proptest::Stream<ShrinkableBase> {
            return shrinks->template transform<ShrinkableBase,ShrinkableBase>([transformer](const ShrinkableBase& shr) -> ShrinkableBase {
                    return shr.map<U, T>(transformer);
                });
            }
        );
    }

    template <typename U, typename T>
    ShrinkableBase flatMap(Function<ShrinkableBase(const T&)> transformer) const {
        return transformer(value.getRef<T>()).with([shrinks = this->shrinks, transformer]() { return shrinks->template transform<ShrinkableBase,ShrinkableBase>([transformer](const ShrinkableBase& shr) -> ShrinkableBase {
                    return shr.flatMap<U, T>(transformer);
                });
            }
        );
    }

    template <typename U, typename T>
    ShrinkableBase mapShrinkable(Function<ShrinkableBase(const ShrinkableBase&)> transformer) const;

    // provide filtered generation, shrinking
    template <typename T>
    ShrinkableBase filter(Function<bool(const T&)> criteria) const {
        // criteria must be true for head
        if(!criteria(value.getRef<T>()))
            throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");
        else
            return with(shrinks->template filter<ShrinkableBase>([criteria](const ShrinkableBase& shr) -> bool {
                return criteria(shr.getRef<T>());
            }).template transform<ShrinkableBase,ShrinkableBase>([criteria](const ShrinkableBase& shr) {
                return shr.filter<T>(criteria);
            }));
    }

    // provide filtered generation, shrinking
    template <typename T>
    ShrinkableBase filter(Function<bool(const T&)> criteria, int tolerance) const {

        static Function<StreamType(const StreamType&,Function<bool(const T&)>, int)> filterStream = +[](const StreamType& stream, Function<bool(const T&)> _criteria, int _tolerance) {
            if(stream.isEmpty())
                return StreamType::empty();
            else {
                for(auto itr = stream.template iterator<ShrinkableBase>(); itr.hasNext();) {
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

        return with(filterStream(getShrinks(), criteria, tolerance).template transform<ShrinkableBase,ShrinkableBase>([criteria, tolerance](const ShrinkableBase& shr) {
            return shr.filter<T>(criteria, tolerance);
        }));
    }

    // concat: continues with then after horizontal dead end
    ShrinkableBase concatStatic(const StreamType& then) const {
        auto shrinksWithThen = shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.concatStatic(then);
        });
        return with(shrinksWithThen.concat(then));
    }

    // concat: extend shrinks stream with function taking parent as argument
    ShrinkableBase concat(Function<StreamType(const ShrinkableBase&)> then) const {
        return with([copy = *this, shrinks = this->shrinks, then]() {
            auto shrinksWithThen = shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.concat(then);
            });
            return shrinksWithThen.concat([=]() { return then(copy); });
        });
    }

    // andThen: continues with then after vertical dead end
    ShrinkableBase andThenStatic(const StreamType& then) const {
        if(shrinks->isEmpty())
            return with(then);
        else
            return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.andThenStatic(then);
            }));
    }

    ShrinkableBase andThen(Function<StreamType(const ShrinkableBase&)> then) const {
        if(shrinks->isEmpty())
            return with(then(*this));
        else
            return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.andThen(then);
            }));
    }

    ShrinkableBase take(int n) const {
        return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([n](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.take(n);
        }));
    }

private:
    ShrinkableBase(const Any& _value, const Lazy<StreamType>& _shrinks) : value(_value), shrinks(_shrinks) {
    }

    Any value;
    Lazy<StreamType> shrinks;

public:

    template <typename T>
    friend struct Shrinkable;

#ifdef PROPTEST_UNTYPED_SHRINKABLE
    template <typename T, typename... ARGS>
    friend Shrinkable<T> proptest::make_shrinkable(ARGS&&... args);
#endif
};

template <typename T>
struct Shrinkable : public ShrinkableBase
{
    using type = T;

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

    template <same_as<T> TT=T> T get() const { return ShrinkableBase::get<T>(); }
    template <same_as<T> TT=T> const T& getRef() const { return ShrinkableBase::getRef<T>(); }
    template <same_as<T> TT=T> T& getMutableRef() { return ShrinkableBase::getMutableRef<T>(); }
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

template <typename T> using Shrinkable = untyped::Shrinkable<T>;

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

template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.template getRef<T>() < rhs.template getRef<T>();
    }
};

template <typename T, typename U>
class less<proptest::Shrinkable<pair<T, U>>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<pair<T,U>>& lhs, const proptest::Shrinkable<pair<T,U>>& rhs) const
    {
        return lhs.template getRef<pair<T,U>>().first < rhs.template getRef<pair<T,U>>().first;
    }
};


}  // namespace std

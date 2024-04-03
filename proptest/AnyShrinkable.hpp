#pragma once

#include "proptest/util/any.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

// based on Shrinkable<Any>
struct AnyShrinkable {
    using StreamElementType = AnyShrinkable;
    using StreamType = Stream;

    AnyShrinkable(const ShrinkableAny& shr)  : shrinkableAny(shr) {}

    template <typename T>
    AnyShrinkable(const Shrinkable<T>& shr)  : shrinkableAny(shr.template map<Any>([](const T& value) { return Any(value); })) {}

    AnyShrinkable clear() const { return AnyShrinkable(shrinkableAny.clear());}

    AnyShrinkable with(const StreamType& otherShrinks) const {
        return AnyShrinkable(shrinkableAny.with(otherShrinks.template transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
            return shr.shrinkableAny;
        })));
    }

    AnyShrinkable with(Function<StreamType()> otherStream) const {
        return AnyShrinkable(shrinkableAny.with([otherStream]() -> ShrinkableAny::StreamType {
            return otherStream().template transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
                return shr.shrinkableAny;
            });
        }));
    }

    template <typename T>
    T get() const { return shrinkableAny.getRef().getRef<T>(); }

    template <typename T>
    const T& getRef() const { return shrinkableAny.getRef().getRef<T>();}

    Any getAny() const {
        return shrinkableAny.getRef();
    }

    StreamType getShrinks() const {
        return shrinkableAny.getShrinks().template transform<AnyShrinkable,ShrinkableAny::StreamElementType>([](const ShrinkableAny::StreamElementType& shr) -> AnyShrinkable {
            return AnyShrinkable(shr);
        });
    }

    template <typename T, typename U>
    AnyShrinkable map(Function<U(const T&)> transformer) const {
        return AnyShrinkable(shrinkableAny.template map<Any>([transformer](const Any& any) -> Any {
            return Any(transformer(any.getRef<T>()));
        }));
    }

    template <typename T, typename U>
    AnyShrinkable flatMap(Function<Shrinkable<U>(const T&)> transformer) const {
        return AnyShrinkable(shrinkableAny.template flatMap<Any>([transformer](const Any& any) -> Shrinkable<U>{
            return transformer(any.getRef<T>());
        }));
    }

    template <typename U>
    AnyShrinkable mapShrinkable(Function<AnyShrinkable(const AnyShrinkable&)> transformer) const;

    // provide filtered generation, shrinking
    template <typename T>
    AnyShrinkable filter(Function<bool(const T&)> criteria) const {
        return AnyShrinkable(shrinkableAny.filter([criteria](const Any& any) -> bool {
            return criteria(any.getRef<T>());
        }));
    }

    // provide filtered generation, shrinking
    template <typename T>
    AnyShrinkable filter(Function<bool(const T&)> criteria, int tolerance) const {
        return AnyShrinkable(shrinkableAny.filter([criteria](const Any& any) -> bool {
            return criteria(any.getRef<T>());
        }, tolerance));
    }

    // concat: continues with then after horizontal dead end
    AnyShrinkable concatStatic(const StreamType& then) const {
        return AnyShrinkable(shrinkableAny.concatStatic(then.template transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
            return shr.shrinkableAny;
        })));
    }

    // concat: extend shrinks stream with function taking parent as argument
    AnyShrinkable concat(Function<StreamType(const AnyShrinkable&)> then) const {
        return AnyShrinkable(shrinkableAny.concat([then](const ShrinkableAny& parent) -> ShrinkableAny::StreamType {
            return then(AnyShrinkable(parent)).transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
                return shr.shrinkableAny;
            });
        }));
    }

    // andThen: continues with then after vertical dead end
    AnyShrinkable andThenStatic(const StreamType& then) const {
        return AnyShrinkable(shrinkableAny.andThenStatic(then.template transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
            return shr.shrinkableAny;
        })));
    }

    AnyShrinkable andThen(Function<StreamType(const AnyShrinkable&)> then) const {
        return AnyShrinkable(shrinkableAny.andThen([then](const ShrinkableAny& parent) -> ShrinkableAny::StreamType {
            return then(AnyShrinkable(parent)).transform<ShrinkableAny::StreamElementType,AnyShrinkable>([](const AnyShrinkable& shr) -> ShrinkableAny::StreamElementType {
                return shr.shrinkableAny;
            });
        }));
    }

    AnyShrinkable take(int n) const {
        return AnyShrinkable(shrinkableAny.take(n));
    }

    ShrinkableAny getShrinkableAny() const { return shrinkableAny; }

private:
    ShrinkableAny shrinkableAny;
};

} // namespace proptest

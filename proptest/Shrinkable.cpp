#include "proptest/Shrinkable.hpp"

namespace proptest {

namespace untyped {

ShrinkableBase::ShrinkableBase(Any _value) : value(_value), shrinks(StreamType::empty()) {}

ShrinkableBase::ShrinkableBase(const Any& _value, const Lazy<StreamType>& _shrinks) : value(_value), shrinks(_shrinks) { }

ShrinkableBase ShrinkableBase::clear() const {
    return ShrinkableBase{value};
}

ShrinkableBase ShrinkableBase::with(const StreamType& otherShrinks) const {
    return ShrinkableBase(value, otherShrinks);
}

ShrinkableBase ShrinkableBase::with(Function<StreamType()> otherStream) const {
    return ShrinkableBase(value, Lazy<StreamType>(otherStream));
}

Any ShrinkableBase::getAny() const { return value; }

ShrinkableBase ShrinkableBase::clone() const {
    return ShrinkableBase(value.clone(), shrinks);
}

ShrinkableBase::StreamType ShrinkableBase::getShrinks() const { return *shrinks; }

// concat: continues with then after horizontal dead end
ShrinkableBase ShrinkableBase::concatStatic(const StreamType& then) const {
    auto shrinksWithThen = shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
        return shr.concatStatic(then);
    });
    return with(shrinksWithThen.concat(then));
}

// concat: extend shrinks stream with function taking parent as argument
ShrinkableBase ShrinkableBase::concat(Function<StreamType(const ShrinkableBase&)> then) const {
    return with([copy = *this, shrinks = this->shrinks, then]() {
        auto shrinksWithThen = shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.concat(then);
        });
        return shrinksWithThen.concat([=]() { return then(copy); });
    });
}

// andThen: continues with then after vertical dead end
ShrinkableBase ShrinkableBase::andThenStatic(const StreamType& then) const {
    if(shrinks->isEmpty())
        return with(then);
    else
        return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.andThenStatic(then);
        }));
}

ShrinkableBase ShrinkableBase::andThen(Function<StreamType(const ShrinkableBase&)> then) const {
    if(shrinks->isEmpty())
        return with(then(*this));
    else
        return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.andThen(then);
        }));
}

ShrinkableBase ShrinkableBase::take(int n) const {
    return with(shrinks->template transform<ShrinkableBase,ShrinkableBase>([n](const ShrinkableBase& shr) -> ShrinkableBase {
        return shr.take(n);
    }));
}

} // namespace typed

} // namespace proptest
#include "proptest/Shrinkable.hpp"

namespace proptest {

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

ShrinkableBase ShrinkableBase::map(Function1 transformer) const {
    return ShrinkableBase(transformer(value)).with([shrinks = this->shrinks, transformer]() -> StreamType {
        return shrinks->template transform<StreamElementType,StreamElementType>([transformer](const ShrinkableBase& shr) -> StreamElementType {
                return shr.map(transformer);
            });
        }
    );
}

ShrinkableBase ShrinkableBase::flatMap(Function1 transformer) const {
    return transformer(value).template getRef<ShrinkableBase>(true).with([shrinks = this->shrinks, transformer]() { return shrinks->template transform<ShrinkableBase,ShrinkableBase>([transformer](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.flatMap(transformer);
            });
        }
    );
}

ShrinkableBase ShrinkableBase::filter(Function1 criteria) const {
    // criteria must be true for head
    if(!criteria(value).template getRef<bool>())
        throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");
    else
        return with(shrinks->template filter<ShrinkableBase>([criteria](const ShrinkableBase& shr) -> bool {
            return criteria(shr.getAny()).template getRef<bool>();
        }).template transform<ShrinkableBase,ShrinkableBase>([criteria](const ShrinkableBase& shr) {
            return shr.filter(criteria);
        }));
}

ShrinkableBase ShrinkableBase::filter(Function1 criteria, int tolerance) const {

    static Function<StreamType(const StreamType&, Function1, int)> filterStream = +[](const StreamType& stream, Function1 _criteria, int _tolerance) {
        if(stream.isEmpty())
            return StreamType::empty();
        else {
            for(auto itr = stream.template iterator<ShrinkableBase>(); itr.hasNext();) {
                auto shr = itr.next();
                auto tail = itr.stream;
                if(_criteria(shr.getAny()).template getRef<bool>()) {
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
    if(!criteria(value).template getRef<bool>())
        throw invalid_argument(__FILE__, __LINE__, "cannot apply criteria");

    return with(filterStream(getShrinks(), criteria, tolerance).template transform<ShrinkableBase,ShrinkableBase>([criteria, tolerance](const ShrinkableBase& shr) {
        return shr.filter(criteria, tolerance);
    }));
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

} // namespace proptest

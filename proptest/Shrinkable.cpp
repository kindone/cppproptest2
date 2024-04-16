#include "proptest/Shrinkable.hpp"

namespace proptest {

ShrinkableBase::ShrinkableBase(const Any& _value) : value(_value), shrinksGen(+[]() { return StreamType::empty(); }) {}

ShrinkableBase::ShrinkableBase(const Any& _value, const Function<StreamType()>& _shrinksGen) : value(_value), shrinksGen(_shrinksGen) { }

ShrinkableBase ShrinkableBase::clear() const {
    return ShrinkableBase{value};
}

ShrinkableBase ShrinkableBase::with(const StreamType& otherShrinks) const {
    return ShrinkableBase(value, Function<StreamType()>([otherShrinks]() { return otherShrinks; }));
}

ShrinkableBase ShrinkableBase::with(Function<StreamType()> otherStreamGen) const {
    return ShrinkableBase(value, otherStreamGen);
}

Any ShrinkableBase::getAny() const { return value; }

ShrinkableBase ShrinkableBase::clone() const {
    return ShrinkableBase(value.clone(), shrinksGen);
}

// must be a callable with signature U(T&)
ShrinkableBase ShrinkableBase::map(Function1 transformer) const {
    return ShrinkableBase(transformer(value)).with([shrinksGen = this->shrinksGen, transformer]() -> StreamType {
        return shrinksGen().template transform<StreamElementType,StreamElementType>([transformer](const ShrinkableBase& shr) -> StreamElementType {
                return shr.map(transformer);
            });
        }
    );
}

// must be a callable with signature Shrinkable<U>(T&)
ShrinkableBase ShrinkableBase::flatMap(Function1 transformer) const {
    return transformer(value).template getRef<ShrinkableBase>(true).with([shrinksGen = this->shrinksGen, transformer]() {
        return shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([transformer](const ShrinkableBase& shr) -> StreamElementType {
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
        return with([shrinksGen = this->shrinksGen, criteria]() {
            return shrinksGen().template filter<ShrinkableBase>([criteria](const ShrinkableBase& shr) -> bool {
                return criteria(shr.getAny()).template getRef<bool>();
            }).template transform<ShrinkableBase,ShrinkableBase>([criteria](const ShrinkableBase& shr) {
                return shr.filter(criteria);
            });
        });
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

    return with(filterStream(shrinksGen(), criteria, tolerance).template transform<ShrinkableBase,ShrinkableBase>([criteria, tolerance](const ShrinkableBase& shr) {
        return shr.filter(criteria, tolerance);
    }));
}

ShrinkableBase::StreamType ShrinkableBase::getShrinks() const { return shrinksGen(); }

// concat: continues with then after horizontal dead end
ShrinkableBase ShrinkableBase::concatStatic(const StreamType& then) const {
    auto shrinksWithThen = shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
        return shr.concatStatic(then);
    });
    return with(shrinksWithThen.concat(then));
}

// concat: extend shrinksGen stream with function taking parent as argument
ShrinkableBase ShrinkableBase::concat(Function<StreamType(const ShrinkableBase&)> then) const {
    return with([copy = *this, then]() {
        auto shrinksWithThen = copy.shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.concat(then);
        });
        return shrinksWithThen.concat([=]() { return then(copy); });
    });
}

// andThen: continues with then after vertical dead end
ShrinkableBase ShrinkableBase::andThenStatic(const StreamType& then) const {
    if(shrinksGen().isEmpty())
        return with(then);
    else
        return with(shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
            return shr.andThenStatic(then);
        }));
}

ShrinkableBase ShrinkableBase::andThen(Function<StreamType(const ShrinkableBase&)> then) const {
    if(shrinksGen().isEmpty())
        return with([copy = *this, then]() { return then(copy); });
    else
        return with([shrinksGen = this->shrinksGen, then]() { return shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([then](const ShrinkableBase& shr) -> ShrinkableBase {
                return shr.andThen(then);
            });
        });
}

ShrinkableBase ShrinkableBase::take(int n) const {
    return with(shrinksGen().template transform<ShrinkableBase,ShrinkableBase>([n](const ShrinkableBase& shr) -> ShrinkableBase {
        return shr.take(n);
    }));
}

} // namespace proptest

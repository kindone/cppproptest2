#include "proptest/shrinker/bool.hpp"
#include "proptest/TypedStream.hpp"

namespace proptest {

Shrinkable<bool> shrinkBool(bool value) {
    if (value) {
        return make_shrinkable<bool>(value).with(TypedStream<Shrinkable<bool>>::one(make_shrinkable<bool>(false)));
    } else {
        return make_shrinkable<bool>(value);
    }
}

} // namespace proptest
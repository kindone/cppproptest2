#include "proptest/shrinker/bool.hpp"
#include "proptest/Stream.hpp"

namespace proptest {

Shrinkable<bool> shrinkBool(bool value) {
    if (value) {
        return Shrinkable<bool>(value, +[]() { return Stream::one<ShrinkableBase>(make_shrinkable<bool>(false)); });
    } else {
        return make_shrinkable<bool>(value);
    }
}

} // namespace proptest

#include "proptest/Generator.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

ShrinkableBase AnyGenerator::operator()(Random& rand) const {
    return anyGen(rand);
}

} // namespace proptest

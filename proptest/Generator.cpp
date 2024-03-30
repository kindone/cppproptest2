#include "proptest/Generator.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

ShrinkableAny AnyGenerator::operator()(Random& rand) const {
    return anyGen(rand);
}

// template struct Function<ShrinkableAny(Random&)>;

} // namespace proptest
#include "proptest/Generator.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

GeneratorCommon::~GeneratorCommon() {}

ShrinkableAny AnyGenerator::operator()(Random& rand) const {
    return anyGen(rand);
}

// template struct Function<ShrinkableAny(Random&)>;

} // namespace proptest

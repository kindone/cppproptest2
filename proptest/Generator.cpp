#include "proptest/Generator.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"

namespace proptest {

GeneratorCommon::~GeneratorCommon() {}

GeneratorCommon GeneratorCommon::filter(Function1 gen, Function1 criteria) {
    return proptest::filter(gen, criteria);
}

GeneratorCommon GeneratorCommon::map(Function1 gen, Function1 mapper)
{
    return proptest::transform(gen, mapper);
}

GeneratorCommon GeneratorCommon::pairWith(Function1 gen, Function1 genFactory)
{
    return proptest::dependency(gen, genFactory);
}

GeneratorCommon GeneratorCommon::flatMap(Function1 gen, Function1 genFactory)
{
    return proptest::derive(gen, genFactory);
}

ShrinkableAny AnyGenerator::operator()(Random& rand) const {
    return anyGen(rand);
}

// template struct Function<ShrinkableAny(Random&)>;

} // namespace proptest

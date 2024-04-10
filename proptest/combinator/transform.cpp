#include "proptest/combinator/transform.hpp"
#include "proptest/Generator.hpp"

namespace proptest {

namespace util {

GeneratorCommon transformImpl(Function1 gen, Function1 transformer)
{
    return GeneratorCommon([gen, transformer](Random& rand) {
        ShrinkableBase shrinkable = gen(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>(true);
        return shrinkable.map(transformer);
    });
}

} // namespace util

} // namespace proptest

#include "proptest/combinator/transform.hpp"
#include "proptest/Generator.hpp"

namespace proptest {

namespace util {

GeneratorCommon transformImpl(Function1<ShrinkableBase> gen, Function1<Any> transformer)
{
    return GeneratorCommon([gen, transformer](Random& rand) {
        ShrinkableBase shrinkable = gen.callDirect(rand);
        return shrinkable.map(transformer);
    });
}

} // namespace util

} // namespace proptest

#include "proptest/combinator/filter.hpp"

namespace proptest {

namespace util {

GeneratorCommon filterImpl(Function1<ShrinkableBase> gen, Function1<bool> criteria)
{
    return GeneratorCommon([gen, criteria](Random& rand) {
        // TODO: add some configurable termination criteria (e.g. maximum no. of attempts)
        while (true) {
            ShrinkableBase shrinkable = gen.callDirect(rand);
            if (criteria(shrinkable.getAny())) {
                return shrinkable.filter(criteria, 1);  // 1: tolerance
            }
        }
    });
}

} // namespace util

} // namespace proptest

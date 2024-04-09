#include "proptest/combinator/filter.hpp"

namespace proptest {

GeneratorCommon filter(Function1 gen, Function1 criteria)
{
    return GeneratorCommon([gen, criteria](Random& rand) {
        // TODO: add some configurable termination criteria (e.g. maximum no. of attempts)
        while (true) {
            ShrinkableBase shrinkable = gen(util::make_any<Random&>(rand)).template getRef<ShrinkableBase>(true);
            if (criteria(shrinkable.getAny()).template getRef<bool>()) {
                return shrinkable.filter(criteria, 1);  // 1: tolerance
            }
        }
    });
}

} // namespace proptest

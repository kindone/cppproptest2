#include "proptest/combinator/accumulate.hpp"

namespace proptest {
namespace util {

GeneratorCommon accumulateImpl(Function1 gen1, Function1 gen2gen, size_t minSize, size_t maxSize)
{
    return interval<uint64_t>(minSize, maxSize).flatMap<int>([gen1, gen2gen](const uint64_t& size) -> Generator<int> {
        return Function1([gen1, gen2gen, size](Random& rand) {
            ShrinkableBase shr = gen1.callDirect(rand).getRef<ShrinkableBase>(true);
            for (size_t i = 0; i < size; i++) {
                Function1 gen2 = gen2gen(shr.getAny()).getRef<Function1>();
                shr = gen2.callDirect(rand).getRef<ShrinkableBase>(true);
            }
            return shr;
        });
    });
}

} // namespace util
} // namespace proptest

#include "proptest/generator/unicode.hpp"
#include "proptest/Generator.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/combinator/oneof.hpp"

namespace proptest {

Shrinkable<uint32_t> UnicodeGen::operator()(Random& rand)
{
    auto frontSize = 0xD7FF - 0x1 + 1;
    auto rearSize = 0x10FFFF - 0xE000 + 1;
    auto size = frontSize + rearSize;
    auto frontGen = weightedGen<uint32_t>(interval<uint32_t>(0x1, 0xD7FF), static_cast<double>(frontSize) / size);
    auto rearGen = weightedGen<uint32_t>(interval<uint32_t>(0xE000, 0x10FFFF), static_cast<double>(rearSize) / size);
    static auto gen = oneOf<uint32_t>(frontGen, rearGen);
    return gen(rand);
}

}  // namespace proptest

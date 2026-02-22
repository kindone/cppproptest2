#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, vector)
{
    Random rand(1);
    auto numGen = gen::int64();
    auto gen1 = gen::vector<int64_t>();
    auto gen2 = gen::vector<int64_t>(numGen);
    auto gen3 = gen::vector<int64_t>(generator(numGen));
    gen1(rand);
    gen2(rand);
    gen3(rand);

    // Config-based constructor (named parameters)
    auto gen4 = gen::vector<int64_t>({.minSize = 5, .maxSize = 20});
    auto gen5 = gen::vector<int64_t>({.elemGen = gen::int64(), .minSize = 1, .maxSize = 10});
    gen4(rand);
    gen5(rand);
}

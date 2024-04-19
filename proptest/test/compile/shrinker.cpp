#include "proptest/shrinker/bool.hpp"
#include "proptest/shrinker/integral.hpp"
#include "proptest/shrinker/string.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Compile, shrinker_bool)
{
    shrinkBool(true);
}

TEST(Compile, shrinker_int)
{
    shrinkIntegral(1);
}

TEST(Compile, shrinker_string)
{
    shrinkString("hello");
}

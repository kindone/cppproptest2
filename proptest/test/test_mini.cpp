#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Simple, property)
{
    forAll([](int a, int b, double c) {
        PROP_STAT(a > 0);
        PROP_STAT(b > 0);
        PROP_STAT(c > 0);
    });
}

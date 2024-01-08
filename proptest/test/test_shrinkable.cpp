#include "proptest/Shrinkable.hpp"
#include "gtest/gtest.h"

using namespace proptest;

TEST(Shrinkable, basic)
{
    Shrinkable<int> shr(100);
}

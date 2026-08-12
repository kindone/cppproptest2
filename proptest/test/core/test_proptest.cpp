#include "proptest/proptest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(proptest, basic)
{
    EXPECT_FOR_ALL([](int a, int b) {
        cout << "a: " << a << ", b: " << b << endl;
    });
}

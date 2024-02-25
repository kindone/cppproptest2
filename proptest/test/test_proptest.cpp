#include "proptest/proptest.hpp"
#include "proptest/test/testutil.hpp"

using namespace proptest;

TEST(proptest, basic)
{
    forAll([](int a, int b) {
        cout << "a: " << a << ", b: " << b << endl;
    });
}

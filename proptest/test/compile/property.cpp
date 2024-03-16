#include "proptest/proptest.hpp"

#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Compile, Property)
{
    auto prop = property([](int i, float f) {
        cout << "matrix: " << i << ", " << f << endl;
    });

    prop.matrix({1,2,3}, {0.2f, 0.3f});

    forAll([](vector<int64_t>) {

    });
}

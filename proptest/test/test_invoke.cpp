#include "proptest/util/matrix.hpp"
#include "proptest/std/io.hpp"
#include "proptest/test/gtest.hpp"

using namespace proptest;

TEST(Matrix, basic)
{
    //auto func = [](int a, double b, string c) {
    Function<int(int,double,string)> func = [](int a, double b, string c) {
        cout << a << ", " << b << ", " << c << endl;
        return a;
    };
    util::cartesianProduct(func, {1,2,3},
       {4.0, 5.0},
       {string("a"), string("b")});
}

#include "proptest/util/matrix.hpp"
#include "proptest/util/invoke.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/floating.hpp"
#include "proptest/generator/string.hpp"
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

    util::cartesianProduct(func, {1},
       {4.0},
       {string("a")});

}

TEST(invoke, basic)
{
    Function<int(int,double,string)> func = [](int a, double b, string c) {
        cout << a << ", " << b << ", " << c << endl;
        return a;
    };
    util::invokeExplicit(func, Arbi<int>(), Arbi<double>(), Arbi<string>());
    util::invoke(func);
    util::invoke(func, Arbi<int>());
}

TEST(invoke, void)
{
    Function<void(int,double,string)> func = [](int a, double b, string c) {
        cout << a << ", " << b << ", " << c << endl;
    };
    util::invokeExplicit(func, Arbi<int>(), Arbi<double>(), Arbi<string>());
    util::invoke(func);
    util::invoke(func, Arbi<int>());
}

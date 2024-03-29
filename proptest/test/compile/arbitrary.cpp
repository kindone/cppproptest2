#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


struct MyObj {
    MyObj(int _a) : a(_a) {}
    int a;
};

struct MyObj2 {
    MyObj2(int _a, int _b) : a(_a), b(_b) {}
    int a;
    int b;
};

using namespace proptest;

// namespace proptest {
// DEFINE_ARBITRARY(MyObj, []() {
//     auto intGen = interval(10, 20);
//     return construct<MyObj, int>(intGen);
// });
// }

// TEST(Compile, define_arbitrary)
// {
//     Random rand(1);
//     auto myObjGen = Arbi<MyObj>();
//     auto shr = myObjGen(rand);
//     cout << shr.get().a << endl;
// }

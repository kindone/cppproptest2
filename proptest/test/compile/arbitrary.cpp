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


TEST(Compile, define_arbitrary_container_without_element_arbitrary)
{
    // Arbi<vector<MyObj>> should works even if Arbi<MyObj> is not defined
    Random rand(1);
    auto myObjGen = interval(10, 20).map<MyObj>([](int a) { return MyObj(a); });
    auto myObjsGen = Arbi<vector<MyObj>>(myObjGen, 1,2);
    auto shr = myObjsGen(rand);

    cout << shr.get()[0].a << endl;
}

namespace proptest {
DEFINE_ARBITRARY(MyObj, []() {
    auto intGen = interval(10, 20);
    return intGen.map<MyObj>([](int i) { return MyObj(i); });
});
}

TEST(Compile, using_defined_arbitrary)
{
    Random rand(1);
    auto myObjGen = Arbi<MyObj>();
    auto shr = myObjGen(rand);
    cout << shr.get().a << endl;
}

// TEST(Compile, using_undefined_arbitrary)
// {
//     Random rand(1);
//     auto myObjGen = Arbi<MyObj2>();
//     auto shr = myObjGen(rand);
//     cout << shr.get().a << endl;
// }


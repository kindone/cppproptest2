#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Example, DefaultGen)
{
    forAll([]([[maybe_unused]] int i, [[maybe_unused]] double d, [[maybe_unused]] string str, [[maybe_unused]] vector<uint64_t> vec) {
        // i, d .....
    });
}

TEST(Example, CustomGen)
{
    auto intGen = gen::elementOf<int>(2, 4, 6);

    forAll([](int num) {
        cout << num << endl;
    }, intGen);
}

TEST(Example, property)
{
    auto prop = property([](string name, int num) {
        cout << "name: " << name << ", num: " << num << endl;
    });

    prop.forAll();
    prop.matrix({"Alice", "Bob"}, {0, 1});
    prop.example("Charlie", 2);
}


TEST(Example, TemplatedGen)
{
    auto intGen = gen::interval<int>(2, 100000);
    auto stringIntGen = intGen.map([](const int& num) {
        return to_string(num);/// "2", "4", "6"
    });

    forAll([]([[maybe_unused]] string str, vector<string> numStrings) {
        //cout << str << endl;
        cout << "[ ";
        if(numStrings.size() > 0)
            cout << numStrings[0];
        for(size_t i = 1; i < numStrings.size(); i++)
            cout << ", " << numStrings[i];
        cout << " ]" << endl;

    }, stringIntGen, gen::vector<string>(stringIntGen) );
}

TEST(Example, MapGen)
{
    auto intGen = gen::elementOf<int>(2, 4, 6);
    auto stringIntGen = intGen.map([](const int& num) {
        return to_string(num);
    });

    auto pairGen = gen::pair(intGen, stringIntGen);

    gen::map<int, string> mapGen;

    forAll([]([[maybe_unused]] string str, [[maybe_unused]] map<int, string> nameAgeMap) {
        //cout << str << endl;
    }, stringIntGen, mapGen.setPairGen(pairGen).setMaxSize(3));
}

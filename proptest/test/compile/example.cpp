#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

TEST(Example, DefaultGen)
{
    forAll([]([[maybe_unused]] int i, [[maybe_unused]] double d, [[maybe_unused]] std::string str, [[maybe_unused]] std::vector<uint64_t> vec) {
        // i, d .....
    });
}

TEST(Example, CustomGen)
{
    auto intGen = elementOf<int>(2, 4, 6);

    forAll([](int num) {
        std::cout << num << std::endl;
    }, intGen);
}

TEST(Example, property)
{
    auto prop = property([](std::string name, int num) {
        std::cout << "name: " << name << ", num: " << num << std::endl;
    });

    prop.forAll();
    prop.matrix({"Alice", "Bob"}, {0, 1});
    prop.example("Charlie", 2);
}


TEST(Example, TemplatedGen)
{
    auto intGen = interval<int>(2, 100000);
    auto stringIntGen = intGen.map([](const int& num) {
        return std::to_string(num);/// "2", "4", "6"
    });

    forAll([]([[maybe_unused]] std::string str, std::vector<std::string> numStrings) {
        //std::cout << str << std::endl;
        std::cout << "[ ";
        if(numStrings.size() > 0)
            std::cout << numStrings[0];
        for(size_t i = 1; i < numStrings.size(); i++)
            std::cout << ", " << numStrings[i];
        std::cout << " ]" << std::endl;

    }, stringIntGen, Arbi<std::vector<std::string>>(stringIntGen) );
}

TEST(Example, MapGen)
{
    auto intGen = elementOf<int>(2, 4, 6);
    auto stringIntGen = intGen.map([](const int& num) {
        return std::to_string(num);
    });

    auto pairGen = pairOf(intGen, stringIntGen);

    Arbi<std::map<int, std::string>> mapGen;

    forAll([]([[maybe_unused]] std::string str, [[maybe_unused]] std::map<int, std::string> nameAgeMap) {
        //std::cout << str << std::endl;
    }, stringIntGen, mapGen.setPairGen(pairGen).setMaxSize(3));
}

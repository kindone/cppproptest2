#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/test/testutil.hpp"
#include "proptest/Random.hpp"

using namespace proptest;

// Example struct for demonstration
struct Point {
    int x, y;
    Point(int x, int y) : x(x), y(y) {}

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

TEST(GenNamespace, BasicTypeGenerators)
{
    Random rand(getCurrentTime());

    // Test boolean generator
    auto boolGen = gen::boolean();
    for(int i = 0; i < 10; i++) {
        auto result = boolGen(rand);
        EXPECT_TRUE(result.getRef() == true || result.getRef() == false);
    }

    // Test string generator
    auto stringGen = gen::string();
    for(int i = 0; i < 10; i++) {
        auto result = stringGen(rand);
        EXPECT_LE(result.getRef().size(), Arbi<string>::defaultMaxSize);
    }

    // Test utf8string generator
    auto alphanumericGen = gen::utf8string(gen::unionOf<uint32_t>(
        gen::interval<uint32_t>('A', 'Z'),
        gen::interval<uint32_t>('a', 'z'),
        gen::interval<uint32_t>('0', '9')
    ));

    forAll([]([[maybe_unused]] std::string name, [[maybe_unused]] UTF8String alphanumeric) {
        // Property test with strings
        for(char c : alphanumeric) {
            EXPECT_TRUE(c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c >= '0' && c <= '9');
        }
    }, gen::string(), alphanumericGen);
}

TEST(GenNamespace, NumericRangeGenerators)
{
    Random rand(getCurrentTime());

    // Test interval generator
    auto intervalGen = gen::interval(1, 100);
    for(int i = 0; i < 50; i++) {
        auto result = intervalGen(rand);
        EXPECT_GE(result.getRef(), 1);
        EXPECT_LE(result.getRef(), 100);
    }

    // Test natural generator
    auto naturalGen = gen::natural(1000);
    for(int i = 0; i < 50; i++) {
        auto result = naturalGen(rand);
        EXPECT_GT(result.getRef(), 0);
        EXPECT_LE(result.getRef(), 1000);
    }

    // Test nonnegative generator
    auto nonNegGen = gen::nonNegative(500);
    for(int i = 0; i < 50; i++) {
        auto result = nonNegGen(rand);
        EXPECT_GE(result.getRef(), 0);
        EXPECT_LE(result.getRef(), 500);
    }
}

TEST(GenNamespace, NumericRangesGenerators)
{
    auto multiRangeIntGen = gen::intervals({
        Interval(-100, -1),    // Negative numbers
        Interval(1, 100)       // Positive numbers
    });

    auto multiRangeUintGen = gen::uintervals({
        UInterval(0, 9),       // Single digits
        UInterval(100, 999)    // Three digits
    });

    // Usage in forAll
    forAll([](int64_t multiRange) {
        // Log the generated value for debugging
        if (!((multiRange >= -100 && multiRange <= -1) || (multiRange >= 1 && multiRange <= 100))) {
            std::cerr << "[FAIL int64_t] multiRange=" << multiRange << std::endl;
        }
        ASSERT_TRUE((multiRange >= -100 && multiRange <= -1) || (multiRange >= 1 && multiRange <= 100))
            << "multiRange out of range: " << multiRange;
    }, multiRangeIntGen);

    forAll([](uint64_t multiRange) {
        if (!((multiRange >= 0 && multiRange <= 9) || (multiRange >= 100 && multiRange <= 999))) {
            std::cerr << "[FAIL uint64_t] multiRange=" << multiRange << std::endl;
        }
        ASSERT_TRUE((multiRange >= 0 && multiRange <= 9) || (multiRange >= 100 && multiRange <= 999))
            << "multiRange out of range: " << multiRange;
    }, multiRangeUintGen);
}

TEST(GenNamespace, ContainerGenerators)
{
    Random rand(getCurrentTime());

    // Test vector generator
    auto vectorGen = gen::vector<int>();
    for(int i = 0; i < 10; i++) {
        auto result = vectorGen(rand);
        EXPECT_LE(result.getRef().size(), 200); // Use default max size
    }

    // Test list generator
    auto listGen = gen::list<string>();
    for(int i = 0; i < 10; i++) {
        auto result = listGen(rand);
        EXPECT_LE(result.getRef().size(), 200); // Use default max size
    }

    // Test set generator
    auto setGen = gen::set<int>();
    for(int i = 0; i < 10; i++) {
        auto result = setGen(rand);
        EXPECT_LE(result.getRef().size(), 200); // Use default max size
    }

    // Test map generator
    auto mapGen = gen::map<string, int>();
    for(int i = 0; i < 10; i++) {
        auto result = mapGen(rand);
        EXPECT_LE(result.getRef().size(), 200); // Use default max size
    }
}

TEST(GenNamespace, StringGeneratorsWithCustomElements)
{
    Random rand(getCurrentTime());

    // Test uppercase string generator
    auto uppercaseGen = gen::string(gen::interval<char>('A', 'Z'));
    for(int i = 0; i < 20; i++) {
        auto result = uppercaseGen(rand);
        const auto& str = result.getRef();
        EXPECT_LE(str.size(), 200); // Use default max size
        for(char c : str) {
            EXPECT_GE(c, 'A');
            EXPECT_LE(c, 'Z');
        }
    }

    // Test digit string generator
    auto digitGen = gen::string(gen::interval<char>('0', '9'));
    for(int i = 0; i < 20; i++) {
        auto result = digitGen(rand);
        const auto& str = result.getRef();
        EXPECT_LE(str.size(), 200); // Use default max size
        for(char c : str) {
            EXPECT_GE(c, '0');
            EXPECT_LE(c, '9');
        }
    }
}

TEST(GenNamespace, Combinators)
{
    Random rand(getCurrentTime());

    // Test just combinator
    auto constantGen = gen::just(42);
    for(int i = 0; i < 10; i++) {
        auto result = constantGen(rand);
        EXPECT_EQ(result.getRef(), 42);
    }

    // Test elementOf combinator
    auto choiceGen = gen::elementOf<int>(1, 3, 5, 7, 11, 13, 17, 19);
    for(int i = 0; i < 50; i++) {
        auto result = choiceGen(rand);
        int val = result.getRef();
        EXPECT_TRUE(val == 1 || val == 3 || val == 5 || val == 7 || 
                   val == 11 || val == 13 || val == 17 || val == 19);
    }

    // Test oneOf combinator
    auto unionGen = gen::oneOf<int>(gen::interval(1, 10), gen::interval(100, 110));
    for(int i = 0; i < 50; i++) {
        auto result = unionGen(rand);
        int val = result.getRef();
        EXPECT_TRUE((val >= 1 && val <= 10) || (val >= 100 && val <= 110));
    }
}

TEST(GenNamespace, ObjectConstruction)
{
    Random rand(getCurrentTime());

    // Test construct combinator
    auto pointGen = gen::construct<Point, int, int>(gen::interval(-10, 10), gen::interval(-10, 10));
    for(int i = 0; i < 20; i++) {
        auto result = pointGen(rand);
        const auto& point = result.getRef();
        EXPECT_GE(point.x, -10);
        EXPECT_LE(point.x, 10);
        EXPECT_GE(point.y, -10);
        EXPECT_LE(point.y, 10);
    }
}

TEST(GenNamespace, FilteringAndTransformation)
{
    Random rand(getCurrentTime());

    // Test filter combinator
    auto evenGen = gen::filter<int>(gen::int32(), [](int n) { return n % 2 == 0; });
    for(int i = 0; i < 50; i++) {
        auto result = evenGen(rand);
        EXPECT_EQ(result.getRef() % 2, 0);
    }

    // Test suchThat combinator (alias for filter)
    auto positiveGen = gen::suchThat<int>(gen::int32(), [](int n) { return n > 0; });
    for(int i = 0; i < 50; i++) {
        auto result = positiveGen(rand);
        EXPECT_GT(result.getRef(), 0);
    }

    // Test transform combinator
    auto stringFromIntGen = gen::transform<int, string>(gen::int32(), [](int n) { return to_string(n); });
    for(int i = 0; i < 20; i++) {
        auto result = stringFromIntGen(rand);
        EXPECT_FALSE(result.getRef().empty());
    }
}

TEST(GenNamespace, Dependencies)
{
    Random rand(getCurrentTime());

    // Test dependency combinator
    auto sizeAndVectorGen = gen::dependency<int, vector<int>>(
        gen::interval(1, 5),
        [](int size) { return gen::vector<int>().setSize(size); }
    );

    for(int i = 0; i < 20; i++) {
        auto result = sizeAndVectorGen(rand);
        const auto& pair = result.getRef();
        EXPECT_GE(pair.first, 1);
        EXPECT_LE(pair.first, 5);
        EXPECT_EQ(pair.second.size(), pair.first);
    }
}

TEST(GenNamespace, ComplexNestedStructures)
{
    Random rand(getCurrentTime());

    // Test complex nested structure
    auto complexGen = gen::vector<map<string, vector<int>>>();
    for(int i = 0; i < 10; i++) {
        auto result = complexGen(rand);
        const auto& vec = result.getRef();
        EXPECT_LE(vec.size(), 200); // Use default max size

        for(const auto& map : vec) {
            EXPECT_LE(map.size(), 200); // Use default max size
            for(const auto& pair : map) {
                EXPECT_LE(pair.second.size(), 200); // Use default max size
            }
        }
    }
}

TEST(GenNamespace, PropertyTestExample)
{
    // Test that gen namespace works with forAll
    forAll([](int x, int y) {
        // Property: addition is commutative
        return x + y == y + x;
    }, gen::interval(-100, 100), gen::interval(-100, 100));
}

TEST(GenNamespace, TemplateArgumentDeduction)
{
    Random rand(getCurrentTime());

    // Test that template argument deduction works
    auto intervalGen = gen::interval(1, 100);  // No explicit <int>
    auto naturalGen = gen::natural(1000);      // No explicit <int>
    auto nonNegGen = gen::nonNegative(500);    // No explicit <int>

    for(int i = 0; i < 10; i++) {
        auto result1 = intervalGen(rand);
        EXPECT_GE(result1.getRef(), 1);
        EXPECT_LE(result1.getRef(), 100);

        auto result2 = naturalGen(rand);
        EXPECT_GT(result2.getRef(), 0);
        EXPECT_LE(result2.getRef(), 1000);

        auto result3 = nonNegGen(rand);
        EXPECT_GE(result3.getRef(), 0);
        EXPECT_LE(result3.getRef(), 500);
    }
}

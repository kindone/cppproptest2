#include "proptest/proptest.hpp"
#include "proptest/test/gtest.hpp"


using namespace proptest;

struct NonCopyable
{
    NonCopyable(int a) : a(a) { }
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = delete;
    ~NonCopyable() {
        cout << "~NonCopyable" << endl;
    }
    int a;
};

TEST(Compile, just)
{
    Random rand(3);
    auto gen1 = just(1);
    EXPECT_EQ(gen1(rand).getRef(), 1);

    auto gen2 = just<NonCopyable>(util::make_any<NonCopyable>(2));
    // getRef<NonCopyable>() is not allowed by new contract (non-copyable type)
    EXPECT_EQ(gen2(rand).getRef().a, 2);
}

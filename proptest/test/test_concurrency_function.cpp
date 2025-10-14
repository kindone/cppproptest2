#include "proptest/statefultest.hpp"
#include "proptest/stateful/concurrency_function.hpp"
#include "proptest/test/gtest.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/vector.hpp"
#include "proptest/util/bitmap.hpp"
#include <mutex>

using namespace proptest;
using namespace proptest::concurrent;

using std::mutex;
using std::lock_guard;

// extern mutex& getMutex();

mutex& getMutex()
{
    static mutex mtx;
    return mtx;
}

class ConcurrencyTest : public ::testing::Test {
public:
};

TEST(concurrency_function, WithoutModel)
{
    auto pushBackGen = gen::int32().map<SimpleAction<vector<int>>>([](const int& value) {
        return SimpleAction<vector<int>>([value](vector<int>& obj) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = gen::just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = gen::just(SimpleAction<vector<int>>([](vector<int>& obj) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionGen = gen::oneOf<SimpleAction<vector<int>>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<vector<int>>(gen::vector<int>(), actionGen);
    prop.go();
}

TEST(concurrency_function, WithModel)
{
    struct Model
    {
    };

    auto pushBackGen = gen::int32().map<Action<vector<int>, Model>>([](const int& value) {
        return Action<vector<int>, Model>([value](vector<int>& obj, Model&) {
            // cout << "PushBack(" << value << ")" << endl;
            lock_guard<mutex> guard(getMutex());
            obj.push_back(value);
        });
    });

    auto popBackGen = gen::just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        if (obj.empty())
            return;
        obj.pop_back();
    }));

    auto clearGen = gen::just(Action<vector<int>, Model>([](vector<int>& obj, Model&) {
        lock_guard<mutex> guard(getMutex());
        obj.clear();
    }));

    auto actionGen = gen::oneOf<Action<vector<int>, Model>>(pushBackGen, popBackGen, clearGen);

    auto prop = concurrency<vector<int>, Model>(
        gen::vector<int>(), [](const vector<int>&) { return Model(); }, actionGen);
    prop.setMaxConcurrency(2);
    prop.go();
}

TEST(concurrency_function, bitmap)
{
    using Bitmap = util::Bitmap;

    auto acquireGen = gen::just(SimpleAction<Bitmap>("Acquire", [](Bitmap& bitmap) {
        [[maybe_unused]] int pos = bitmap.acquire();
        bitmap.unacquire(pos);
    }));

    [[maybe_unused]] auto unacquireGen = gen::integers<int>(0, Bitmap::size).map<SimpleAction<Bitmap>>(+[](const int& pos) {
        return SimpleAction<Bitmap>("Unacquire", [pos](Bitmap& bitmap) {
            try {
                bitmap.unacquire(pos);
                cout << "unacquired" << endl;
            } catch(runtime_error&) {
                cout << "failed to unacquire" << endl;
            }
        });
    });

    auto actionGen = gen::oneOf<SimpleAction<Bitmap>>(acquireGen/*, unacquireGen*/);
    auto prop = concurrency<Bitmap>(
        gen::just<Bitmap>(Bitmap()), actionGen);
    prop.go();
}

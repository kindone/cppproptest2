#include <memory>
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"
#include "proptest/Stream.hpp"

namespace proptest {

template <typename T> struct Shrinkable;

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable
{
    using type = T;
    using Stream = Stream<Shrinkable<T>>;

    Shrinkable(Any _value) : value(_value) {}

    Shrinkable& operator=(const Shrinkable& other) {
        value = other.value;
        then = other.then;
        return *this;
    }

    Shrinkable with(const Stream& otherThen) const {
        return Shrinkable(value, otherThen);
    }

    // operator T() const { return get(); }
    T get() const { return value.getRef<T>(); }
    T& getRef() const { return value.getRef<T>(); }
    Any getAny() const { return value; }

    template <typename U>
    Shrinkable<U> map(Function<U(const T&)> transformer) const

    template <typename U>
    Shrinkable<U> flatMap(Function<Shrinkable<U>(const T&)> transformer);

    template <typename U>
    Shrinkable<U> mapShrinkable(Function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const;

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria) const;

    // provide filtered generation, shrinking
    Shrinkable<T> filter(Function<bool(const T&)> criteria, int tolerance);

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(const Stream& then) const;

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(Function<Stream(const Shrinkable<T>&)> then) const;

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(const Stream& then) const;

    Shrinkable<T> andThen(Function<Stream(const Shrinkable<T>&)> then) const;

    Shrinkable<T> take(int n) const;

private:
    Shrinkable(Any _value, const Stream& _then) : value(_value), then(_then) {}

    Any value;
    shared_ptr<Stream> then;

public:

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};


} // namespace proptest
#include "proptest/api.hpp"
#include "proptest/std/lang.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/util/anyfunction.hpp"

#include "proptest/typefwd.hpp"

namespace proptest {

template <typename T> struct Iterator;
template <typename T> struct Stream;

template <typename T>
struct Iterator
{
    Iterator(const Stream<T>& stream) : stream(stream) {}

    virtual ~Iterator() {}
    virtual bool hasNext() {
        return !stream.isEmpty();
    }
    T next() {
        if(!hasNext())
            throw runtime_error("no more elements in stream");

        T value = stream.getHeadRef();
        stream = stream.getTail();
        return value;
    }

    Stream<T> stream;
};


struct StreamBase
{
    StreamBase(const Any& value, const Function<shared_ptr<StreamBase>()>& gen) : head(value), tailGen(gen) {}
    virtual ~StreamBase() {}
    bool isEmpty() const {
        return head.isEmpty();
    };

protected:
    Any head;
    Function<shared_ptr<StreamBase>()> tailGen;

    StreamBase(const shared_ptr<StreamBase>& other) : head(other->head), tailGen(other->tailGen) {}

public:
    static shared_ptr<StreamBase> empty() {
        return util::make_shared<StreamBase>(Any::empty, []() -> shared_ptr<StreamBase> { return StreamBase::empty(); });
    }
};


template <typename T>
struct Stream : public StreamBase
{
    Stream(const Any& value, const Function<shared_ptr<StreamBase>()>& callable) : StreamBase{value, callable} {}

    Stream(const Stream<T>& other) : StreamBase{other.head, other.tailGen} {}
    explicit Stream(const shared_ptr<StreamBase>& other) : StreamBase{other} {}

    const T& getHeadRef() const {
        return head.getRef<T>();
    }

    Stream<T> getTail() const {
        return Stream<T>{tailGen()};
    }

    Iterator<T> iterator() const {
        return Iterator<T>{*this};
    }

    template <typename U>
    shared_ptr<Stream<U>> transform(Function<U(const T&)> transformer) const {
        if(isEmpty())
            return Stream<U>::empty();
        return util::make_shared<Stream<U>>(transformer(head.getRef<T>()), [thisStream = *this, transformer]() -> shared_ptr<StreamBase> {
            return static_pointer_cast<Stream<T>>(thisStream.tailGen())->template transform<U>(transformer);
        });
    }

    shared_ptr<Stream<T>> filter(Function<bool(const T&)> criteria) const {
        if(isEmpty())
            return Stream::empty();
        else {
            for(auto itr = iterator(); itr.hasNext(); ) {
                const T& value = itr.next();
                if(criteria(value)) {
                    auto stream = itr.stream;
                    return util::make_shared<Stream>(value, [stream,criteria]() -> shared_ptr<StreamBase> { return stream.filter(criteria); });
                }
            }
        }
        return Stream::empty();
    }

    shared_ptr<Stream<T>> concat(shared_ptr<StreamBase> other) const {
        if(isEmpty())
            return static_pointer_cast<Stream>(other);
        else {
            return util::make_shared<Stream>(head.getRef<T>(), [stream = *this, other]() -> shared_ptr<StreamBase> { return stream.getTail().concat(other); });
        }
    }

    shared_ptr<StreamBase> concat(const Stream<T>& other) const {
        if(isEmpty())
            return other;
        else {
            return util::make_shared<Stream>(head.getRef<T>(), [stream = *this, other]() -> shared_ptr<StreamBase> { return stream.getTail().concat(other); });
        }
    }

    shared_ptr<Stream<T>> take(int n) const {
        if(isEmpty() || n <= 0)
            return Stream::empty();
        else {
            return util::make_shared<Stream>(head.getRef<T>(), [stream = *this, n]() -> shared_ptr<StreamBase> { return stream.getTail().take(n-1); });
        }
    }


public:

    static shared_ptr<Stream<T>> empty() {
        return static_pointer_cast<Stream<T>>(StreamBase::empty());
    }

    static shared_ptr<Stream<T>> one(const T& value) {
        return util::make_shared<Stream<T>>(Any(value), []() -> shared_ptr<StreamBase> { return StreamBase::empty(); });
    }

    static shared_ptr<Stream<T>> two(const T& value1, const T& value2) {
        Any value2Any(value2);
        return util::make_shared<Stream<T>>(Any(value1), [value2Any]() -> shared_ptr<StreamBase> { return Stream<T>::one(value2Any.getRef<T>()); });
    }
};


} // namespace proptest
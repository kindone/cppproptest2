#include <memory>
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"

namespace proptest {


struct ShrinkableBase {
    virtual ~ShrinkableBase() {}
};

template <typename T>
struct TypedShrinkable : ShrinkableBase{

};

struct UntypedShrinkable {
    template <typename T>
    T get();

    shared_ptr<ShrinkableBase> shrinkableBase;
};

} // namespace proptest
#include <memory>
#include "proptest/typefwd.hpp"
#include "proptest/util/any.hpp"

namespace proptest {

struct GeneratorBase {
    virtual Shrinkable operator()(Random&) = 0;
};

template <typename T>
struct GeneratorImpl : GeneratorBase {

};

template <typename T>
struct Generator {
    virtual Shrinkable operator()(Random& random) {
        return generatorBase->operator()(random);
    }

    shared_ptr<GeneratorBase> generatorBase;
};

}  // namespace proptest
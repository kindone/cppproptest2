#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

// template struct PROPTEST_API Function<Shrinkable<Any>(Random&)>;

#ifndef PROPTEST_UNTYPED_SHRINKABLE

namespace typed {
template struct PROPTEST_API Shrinkable<Any>;
template struct PROPTEST_API Shrinkable<vector<Shrinkable<Any>>>;
template struct PROPTEST_API Shrinkable<pair<Any, Any>>;
}

#endif // PROPTEST_UNTYPED_SHRINKABLE

#ifndef PROPTEST_UNTYPED_STREAM
namespace typed {
template struct PROPTEST_API Stream<Shrinkable<vector<Shrinkable<Any>>>>;
template struct PROPTEST_API Stream<Shrinkable<Any>>;
template struct PROPTEST_API Stream<Shrinkable<pair<Any, Any>>>;
}
#endif


} // namespace proptest

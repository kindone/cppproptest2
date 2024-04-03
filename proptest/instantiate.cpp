#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

#ifndef PROPTEST_UNTYPED_STREAM
namespace typed {
template struct PROPTEST_API Stream<Shrinkable<vector<Shrinkable<Any>>>>;
template struct PROPTEST_API Stream<Shrinkable<Any>>;
template struct PROPTEST_API Stream<Shrinkable<pair<Any, Any>>>;
}
#endif

} // namespace proptest

#include "proptest/Shrinkable.hpp"

namespace proptest {
// explicit instantiation

#ifndef PROPTEST_UNTYPED_SHRINKABLE
namespace typed {
template class PROPTEST_API Shrinkable<Any>;
}
#endif

} // namespace proptest

#include "proptest/stateful/concurrency_function.hpp"

DEFINE_REGISTERED_OBJECT_PRINTER(PROPTEST_API, ConcurrentTestDumpObject, const proptest::concurrent::ConcurrentTestDump,
{
    os << x;
}
);

namespace proptest {
namespace concurrent {

Spawner::~Spawner() {}

} // namespace concurrent
} // namespace proptest

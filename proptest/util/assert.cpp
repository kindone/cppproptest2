#include "proptest/util/assert.hpp"

namespace proptest {
namespace util {

ostream& errorOrEmpty(bool error)
{
    static stringstream str;
    static ostream& empty = str;
    if (error)
        return cerr;
    else
        return empty;
}

}  // namespace util

AssertFailed::~AssertFailed() {}

PropertyFailedBase::~PropertyFailedBase() {}

PropertyFailed::~PropertyFailed() {}


Discard::~Discard() {}

Success::~Success() {}

}  // namespace proptest

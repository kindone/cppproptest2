#include "proptest/util/misc.hpp"

namespace proptest {

namespace util {

IosFlagSaver::IosFlagSaver(ostream& _ios) : ios(_ios), f(_ios.flags()) {}
IosFlagSaver::~IosFlagSaver()
{
    ios.flags(f);
}

} // namespace util

}  // namespace proptest

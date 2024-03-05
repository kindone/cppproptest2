#include "proptest/util/anyfunction.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

template struct Function<ShrinkableAny(Random&)>;
template struct Shrinkable<Any>;
template struct Shrinkable<vector<ShrinkableAny>>;
template struct Shrinkable<pair<Any, Any>>;
template struct Stream<Shrinkable<vector<ShrinkableAny>>>;
template struct Stream<ShrinkableAny>;
template struct Stream<Shrinkable<pair<Any, Any>>>;

} // namespace proptest
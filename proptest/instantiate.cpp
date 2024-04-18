#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Generator.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/string.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/pair.hpp"

namespace proptest {

template struct Shrinkable<bool>;
template struct Shrinkable<int>;
template struct Shrinkable<uint32_t>;
template struct Shrinkable<uint64_t>;
template struct Shrinkable<string>;
template struct Shrinkable<vector<ShrinkableBase>>;

template struct GeneratorBase<bool>;
template struct GeneratorBase<int>;
template struct GeneratorBase<uint32_t>;
template struct GeneratorBase<uint64_t>;
template struct GeneratorBase<string>;

template struct Generator<bool>;
template struct Generator<int>;
template struct Generator<uint32_t>;
template struct Generator<uint64_t>;
template struct Generator<string>;
} // namespace proptest

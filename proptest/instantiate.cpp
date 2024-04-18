#include "proptest/api.hpp"
#include "proptest/util/function.hpp"
#include "proptest/util/any.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Generator.hpp"
#include "proptest/generator/integral.hpp"
#include "proptest/generator/string.hpp"
#include "proptest/generator/bool.hpp"
#include "proptest/generator/floating.hpp"
#include "proptest/generator/utf8string.hpp"
#include "proptest/generator/utf16string.hpp"
#include "proptest/generator/cesu8string.hpp"
#include "proptest/std/vector.hpp"
#include "proptest/std/pair.hpp"
#include "proptest/util/define.hpp"

namespace proptest {

#define INSTANTIATE_ANYVAL(TYPE) INSTANTIATE_STRUCT_TYPE(::proptest::AnyVal, TYPE)
#define INSTANTIATE_ANYREF(TYPE) INSTANTIATE_STRUCT_TYPE(::proptest::AnyRef, TYPE)
#define INSTANTIATE_SHRINKABLE(TYPE) INSTANTIATE_STRUCT_TYPE(::proptest::Shrinkable, TYPE)
#define INSTANTIATE_GENERATORBASE(TYPE) INSTANTIATE_STRUCT_TYPE(::proptest::Generator, TYPE)
#define INSTANTIATE_GENERATOR(TYPE) INSTANTIATE_STRUCT_TYPE(::proptest::GeneratorBase, TYPE)

#define INSTANTIATE_FUNCTION(SIGNATURE) INSTANTIATE_STRUCT_TYPE(::proptest::Function, SIGNATURE)
#define INSTANTIATE_GENERATOR_FUNCTION(TYPE) INSTANTIATE_FUNCTION(Shrinkable<TYPE>(Random&))

DEFINE_FOR_ALL_BASIC_TYPES(INSTANTIATE_ANYVAL);
DEFINE_FOR_ALL_STRINGTYPES(INSTANTIATE_ANYREF);
DEFINE_FOR_ALL_BASIC_TYPES(INSTANTIATE_SHRINKABLE);
INSTANTIATE_SHRINKABLE(vector<ShrinkableBase>);

DEFINE_FOR_ALL_BASIC_TYPES(INSTANTIATE_GENERATOR_FUNCTION);
INSTANTIATE_FUNCTION(ShrinkableBase(Random&));

DEFINE_FOR_ALL_BASIC_TYPES(INSTANTIATE_GENERATORBASE);
DEFINE_FOR_ALL_BASIC_TYPES(INSTANTIATE_GENERATOR);

} // namespace proptest

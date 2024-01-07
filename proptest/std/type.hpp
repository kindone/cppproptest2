#pragma once
#include <type_traits>
#include <typeinfo>

namespace proptest {

using std::decay_t;
using std::is_trivial;
using std::remove_reference;
using std::remove_reference_t;
using std::type_info;
using std::bad_cast;

using std::bool_constant;
using std::false_type;
using std::true_type;

using std::conditional_t;
using std::conjunction_v;

using std::is_convertible_v;
using std::is_fundamental_v;
using std::is_lvalue_reference;
using std::is_pointer;
using std::is_function_v;
using std::is_same;
using std::is_same_v;

using std::enable_if;
using std::enable_if_t;

} // namespace proptest
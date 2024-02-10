#pragma once

namespace proptest {
namespace util {

template <typename RET, typename...ARGS, GenLike...GENS>
decltype(auto) invoke(Function<RET(ARGS...)> func, GENS&&...gens) {

}

} // namespace util
} // namespace proptest

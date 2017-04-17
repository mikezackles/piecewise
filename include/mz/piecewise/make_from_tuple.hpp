#ifndef UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7
#define UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7

#include <tuple>
#include <type_traits>
#include <utility>

#include <iostream>

namespace mz { namespace piecewise {
  namespace detail {
    template <typename ...Args, std::size_t ...Indices, typename Callback>
    auto unpack_tuple(
      std::tuple<Args...> args
    , std::index_sequence<Indices...>
    , Callback callback) {
      return callback(std::forward<Args>(std::get<Indices>(args))...);
    }
  }

  template <typename ...Args, typename Callback>
  auto forward_tuple(std::tuple<Args...> args, Callback callback) {
    return detail::unpack_tuple(
      std::move(args), std::make_index_sequence<sizeof...(Args)>{}, callback
    );
  }
}}

#endif

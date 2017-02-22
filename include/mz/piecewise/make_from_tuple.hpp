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
    , Callback&& callback) {
      return callback(std::forward<Args>(std::get<Indices>(args))...);
    }
  }

  // This function can be used for generic forwarding (e.g., to a factory function)
  template <typename ...Args, typename Callback>
  auto forward_tuple(std::tuple<Args...> args, Callback&& callback) {
    return detail::unpack_tuple(
      std::move(args), std::make_index_sequence<sizeof...(Args)>{}, std::forward<Callback>(callback)
    );
  }

  // Use this in conjunction with std::piecewise_construct,
  // std::piecewise_construct_t, and std::forward_as_tuple to enable perfect
  // forwarding to construct multiple data members. See README.md for more
  // detail.
  template <typename T, typename ...Args>
  auto braced_make_from_tuple(std::tuple<Args...> packed) {
    return forward_tuple(std::move(packed), [](auto&&... args) { return T{std::forward<decltype(args)>(args)...}; });
  }

  template <typename T, typename ...Args>
  auto make_from_tuple(std::tuple<Args...> packed) {
    return forward_tuple(std::move(packed), [](auto&&... args) { return T(std::forward<decltype(args)>(args)...); });
  }
}}

#endif

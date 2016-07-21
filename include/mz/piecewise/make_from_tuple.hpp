#ifndef UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7
#define UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7

#include <tuple>
#include <utility>

namespace mz { namespace piecewise {
  namespace detail {
    template <typename T, typename ...Args, std::size_t ...Indices>
    T make_from_tuple_and_indices(std::tuple<Args...> const& args, std::index_sequence<Indices...>) {
      return T{std::forward<Args>(std::get<Indices>(args))...};
    }
  }

  // Use this in conjunction with std::piecewise_construct,
  // std::piecewise_construct_t, and std::forward_as_tuple to enable perfect
  // forwarding to construct multiple data members. See README.md for more
  // detail.
  template <typename T, typename ...Args>
  T make_from_tuple(std::tuple<Args...> const& args) {
    return detail::make_from_tuple_and_indices<T>(
      args, std::make_index_sequence<sizeof...(Args)>{}
    );
  }
}}

#endif

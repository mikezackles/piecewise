#ifndef UUID_D1091115_FC3B_4BB0_BCB7_0F0137449C49
#define UUID_D1091115_FC3B_4BB0_BCB7_0F0137449C49

#include <tuple>
#include <type_traits>

namespace mz { namespace piecewise { namespace tuple_list {
  template <typename T, typename ...Ts>
  struct split_result {
    std::remove_reference_t<T> head;
    std::tuple<Ts...> tail;
  };

  namespace detail {
    template <typename T, typename ...Ts, std::size_t ...Indices>
    inline auto split_impl(
      std::tuple<T, Ts...> list
    , std::index_sequence<Indices...>
    )-> split_result<T, Ts...> {
      return {
        std::move(std::get<0>(list))
      , std::make_tuple(std::move(std::get<Indices+1>(list))...)
      };
    }

    template <typename ...Ts, typename T, std::size_t ...Indices>
    inline auto combine_impl(
      std::tuple<Ts...> reverse_tail
    , std::index_sequence<Indices...>
    , T reverse_head
    ) {
      return std::make_tuple(
        std::move(std::get<Indices>(reverse_tail))...
      , std::move(reverse_head)
      );
    }
  }

  template <typename T, typename ...Ts>
  inline auto split(std::tuple<T, Ts...> list) {
    return detail::split_impl(
      std::move(list)
    , std::make_index_sequence<sizeof...(Ts)>{}
    );
  }

  template <typename ...Ts, typename T>
  inline auto combine(std::tuple<Ts...> reverse_tail, T reverse_head) {
    return detail::combine_impl(
      std::move(reverse_tail)
    , std::make_index_sequence<sizeof...(Ts)>{}
    , std::move(reverse_head)
    );
  }
}}}

#endif

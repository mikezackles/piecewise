#ifndef UUID_D1091115_FC3B_4BB0_BCB7_0F0137449C49
#define UUID_D1091115_FC3B_4BB0_BCB7_0F0137449C49

#include <tuple>

namespace mz { namespace piecewise { namespace tuple_list {
  template <typename T, typename ...Ts>
  struct split_result {
    decltype(auto) unpack_head() { return std::get<0>(head); }
    std::tuple<T> head;
    std::tuple<Ts...> tail;
  };

  namespace detail {
    template <typename T, typename ...Ts, std::size_t ...Indices>
    static auto split_impl(
      std::tuple<T, Ts...> list
    , std::index_sequence<Indices...>
    )-> split_result<T, Ts...> {
      return {
        std::forward_as_tuple(std::forward<T>(std::get<0>(list)))
      , std::forward_as_tuple(
          std::forward<Ts>(std::get<Indices+1>(list))...
        )
      };
    }

    template <typename T, typename ...Ts, std::size_t ...Indices>
    static auto combine_impl(
      std::tuple<T> head
    , std::tuple<Ts...> tail
    , std::index_sequence<Indices...>
    ) {
      return std::forward_as_tuple(
        std::forward<T>(std::get<0>(head))
      , std::forward<Ts>(std::get<Indices>(tail))...
      );
    }
  }

  template <typename T, typename ...Ts>
  static auto split(std::tuple<T, Ts...> list) {
    return detail::split_impl(
      std::move(list)
    , std::make_index_sequence<sizeof...(Ts)>{}
    );
  }

  template <typename T, typename ...Ts>
  static auto combine(std::tuple<T> head, std::tuple<Ts...> tail) {
    return detail::combine_impl(
      std::move(head)
    , std::move(tail)
    , std::make_index_sequence<sizeof...(Ts)>{}
    );
  }
}}}

#endif

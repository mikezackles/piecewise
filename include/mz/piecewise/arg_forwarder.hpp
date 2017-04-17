#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/make_from_tuple.hpp>

namespace mz { namespace piecewise {
  namespace tuple_list {
    namespace detail {
      template <typename ...Ts, std::size_t ...Indices>
      static auto tail_impl(
        std::tuple<Ts...> list
      , std::index_sequence<Indices...>
      ) {
        return std::forward_as_tuple(
          std::forward<Ts>(std::get<Indices+1>(list))...
        );
      }

      template <typename T, typename ...Ts, std::size_t ...Indices>
      static auto cons_impl(
        T&& head
      , std::tuple<Ts...> tail
      , std::index_sequence<Indices...>
      ) {
        return std::forward_as_tuple(
          std::forward<T>(head)
        , std::forward<Ts>(std::get<Indices>(tail))...
        );
      }
    }

    template <typename ...Ts>
    static decltype(auto) head(std::tuple<Ts...> list) {
      return std::get<0>(list);
    }

    template <typename ...Ts>
    static auto tail(std::tuple<Ts...> list) {
      return detail::tail_impl(
        std::move(list)
      , std::make_index_sequence<sizeof...(Ts)-1>{}
      );
    }

    template <typename T, typename ...Ts>
    static auto cons(T&& head, std::tuple<Ts...> tail) {
      return detail::cons_impl(
        std::forward<T>(head)
      , std::move(tail)
      , std::make_index_sequence<sizeof...(Ts)>{}
      );
    }
  }

  template <typename Callback, typename ...RefTypes>
  class Wrapper {
  public:
    Wrapper(std::tuple<RefTypes...> packed_args_, Callback&& callback_)
      : packed_args{std::move(packed_args_)}
      , callback{std::forward<Callback>(callback_)}
    {}

    template <typename ...Args>
    auto construct(Args&&... args) {
      return forward_tuple(
        std::move(packed_args)
      , std::forward<Callback>(callback)
      , std::forward<Args>(args)...
      );
    }

  private:
    std::tuple<RefTypes...> packed_args;
    Callback&& callback;
  };

  template <typename Callback, typename ...RefTypes>
  static Wrapper<Callback, RefTypes...> make_wrapper(
    std::tuple<RefTypes...> packed_args
  , Callback&& callback
  ) {
    return {std::move(packed_args), std::forward<Callback>(callback)};
  }

  template <typename Callback, typename ...Args>
  static auto forward(Callback&& callback, Args&&... args) {
    return make_wrapper(
      std::forward_as_tuple(std::forward<Args>(args)...)
    , std::forward<Callback>(callback)
    );
  }

  template <
    typename ...ArgPacks
  , typename ...Thunks
  , typename OnSuccess
  , typename OnFail
  > static auto multifail(
    std::tuple<ArgPacks...> arg_packs
  , std::tuple<Thunks...> thunks
  , OnSuccess&& on_success
  , OnFail&& on_fail
  ) {
    return tuple_list::head(arg_packs).construct(
      [ arg_packs = tuple_list::tail(arg_packs)
      , thunks = std::move(thunks)
      , &on_success, &on_fail
      ] (auto thunk) {
        multifail(
          std::move(arg_packs)
        , tuple_list::cons(thunk, thunks)
        , on_success
        , on_fail
        );
      }
    , on_fail
    );
  }

  template <typename ...Thunks, typename OnSuccess, typename OnFail>
  static auto multifail(
    std::tuple<>
  , std::tuple<Thunks...> thunks
  , OnSuccess&& on_success, OnFail&&
  ) {
    return forward_tuple(std::move(thunks), on_success);
  }
}}

#endif

#ifndef UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D
#define UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D

#include <mz/piecewise/make_from_tuple.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mz { namespace piecewise {
  namespace detail {
    template <
      typename ...ArgPacks, typename ...Thunks
    , typename OnSuccess, typename OnFail
    > inline auto multifail_impl(
      std::tuple<ArgPacks...> arg_packs, std::tuple<Thunks...> thunks
    , OnSuccess& on_success, OnFail& on_fail
    ) {
      auto split_arg_packs = tuple_list::split(std::move(arg_packs));
      return split_arg_packs.unpack_head().construct(
        [ arg_packs = std::move(split_arg_packs.tail)
        , thunks = std::move(thunks)
        , &on_success, &on_fail
        ] (auto thunk) {
          multifail(
            std::move(arg_packs)
          , tuple_list::combine(thunk, thunks)
          , on_success, on_fail
          );
        }
      , on_fail
      );
    }

    template <
      typename ...Thunks
    , typename OnSuccess, typename OnFail
    > inline auto multifail_impl(
      std::tuple<>, std::tuple<Thunks...> thunks
    , OnSuccess& on_success, OnFail&
    ) {
      return forward_tuple(std::move(thunks), on_success);
    }
  }

  template <
    typename ...ArgPacks
  , typename OnSuccess, typename OnFail
  > inline auto multifail(
    OnSuccess&& on_success, OnFail&& on_fail
  , ArgPacks&&... arg_packs
  ) {
    return detail::multifail_impl(
      std::forward_as_tuple(std::forward<ArgPacks>(arg_packs)...)
    , std::tuple<>{}
    , on_success, on_fail
    );
  }
}}

#endif
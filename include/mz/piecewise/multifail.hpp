#ifndef UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D
#define UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D

#include <mz/piecewise/forward_tuple.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mz { namespace piecewise {
  namespace detail {
    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ArgPacks, typename Builders
    > struct MultifailImpl;

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ...ArgPacks, typename ...Builders
    > inline auto multifail_impl(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , std::tuple<ArgPacks...> arg_packs, std::tuple<Builders...> builders
    ) {
      return MultifailImpl<
        Constructor
      , OnSuccess, OnFail
      , std::tuple<ArgPacks...>
      , std::tuple<Builders...>
      >{}(
        constructor
      , on_success, on_fail
      , std::move(arg_packs)
      , std::move(builders)
      );
    }

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ArgPacks, typename Builders
    > struct MultifailImpl {
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , ArgPacks arg_packs, Builders builders
      ) const {
        auto split_arg_packs = tuple_list::split(std::move(arg_packs));
        return split_arg_packs.head.construct(
          [ &constructor
          , &on_success, &on_fail
          , arg_packs = std::move(split_arg_packs.tail)
          , builders = std::move(builders)
          ] (auto builder) mutable {
            multifail_impl(
              constructor
            , on_success, on_fail
            , std::move(arg_packs)
            , tuple_list::combine(std::move(builders), std::move(builder))
            );
          }
        , on_fail
        );
      }
    };

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename Builders
    > struct MultifailImpl<
        Constructor
      , OnSuccess, OnFail
      , std::tuple<>
      , Builders
      > {
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail&
      , std::tuple<>, Builders packed_builders
      ) const {
        return forward_tuple(
          [&constructor, &on_success](auto... builders) {
            return on_success(
              builder(constructor, std::move(builders)...)
            );
          }
        , std::move(packed_builders)
        );
      }
    };
  }

  template <
    typename Constructor
  , typename OnSuccess, typename OnFail
  , typename ...ArgPacks
  > inline auto multifail(
    Constructor&& constructor
  , OnSuccess&& on_success, OnFail&& on_fail
  , ArgPacks... arg_packs
  ) {
    return detail::multifail_impl(
      constructor
    , on_success, on_fail
    , std::make_tuple(std::move(arg_packs)...)
    , std::tuple<>{}
    );
  }
}}

#endif

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
    , typename ...RegularArgs
    > inline auto multifail_impl(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , std::tuple<ArgPacks...> arg_packs, std::tuple<Builders...> builders
      , RegularArgs&&... regular_args
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
      , std::forward<RegularArgs>(regular_args)...
      );
    }

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ArgPacks, typename Builders
    > struct MultifailImpl {
      template <typename ...RegularArgs>
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , ArgPacks arg_packs, Builders builders
      , RegularArgs&&... regular_args
      ) const {
        auto split_arg_packs = tuple_list::split(std::move(arg_packs));
        return split_arg_packs.head.construct(
          [ &constructor
          , &on_success, &on_fail
          , arg_packs = std::move(split_arg_packs.tail)
          , builders = std::move(builders)
          , &regular_args...
          ] (auto builder) mutable {
            multifail_impl(
              constructor
            , on_success, on_fail
            , std::move(arg_packs)
            , tuple_list::combine(std::move(builders), std::move(builder))
            , std::forward<RegularArgs>(regular_args)...
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
      template <typename ...RegularArgs>
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail&
      , std::tuple<>, Builders packed_builders
      , RegularArgs&&... regular_args
      ) const {
        return forward_tuple(
          [&constructor, &on_success](auto&&... args) {
            return on_success(
              builder(
                constructor
              , std::forward<decltype(args)>(args)...
              )
            );
          }
        , std::move(packed_builders)
        , std::forward<RegularArgs>(regular_args)...
        );
      }
    };
  }

  template <typename ...Builders>
  inline auto builders(Builders... builders) {
    return std::make_tuple(std::move(builders)...);
  }

  template <
    typename Constructor
  , typename OnSuccess, typename OnFail
  , typename ...ArgPacks
  , typename ...RegularArgs
  > inline auto multifail(
    Constructor&& constructor
  , OnSuccess&& on_success, OnFail&& on_fail
  , std::tuple<ArgPacks...> arg_packs
  , RegularArgs&&... regular_args
  ) {
    return detail::multifail_impl(
      constructor
    , on_success, on_fail
    , std::move(arg_packs)
    , std::tuple<>{}
    , std::forward<RegularArgs>(regular_args)...
    );
  }
}}

#endif

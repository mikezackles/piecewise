#ifndef UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D
#define UUID_5562D1DE_F4CC_4547_8BC3_0D931B26520D

#include <mz/piecewise/forward_tuple.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mz { namespace piecewise {
  namespace detail {
    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ArgPacks, typename Thunks
    > struct MultifailImpl;

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ...ArgPacks, typename ...Thunks
    > inline auto multifail_impl(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , std::tuple<ArgPacks...> arg_packs, std::tuple<Thunks...> thunks
    ) {
      return MultifailImpl<
        Constructor
      , OnSuccess, OnFail
      , std::tuple<ArgPacks...>
      , std::tuple<Thunks...>
      >{}(
        constructor
      , on_success, on_fail
      , std::move(arg_packs)
      , std::move(thunks)
      );
    }

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename ArgPacks, typename Thunks
    > struct MultifailImpl {
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail& on_fail
      , ArgPacks arg_packs, Thunks thunks
      ) const {
        auto split_arg_packs = tuple_list::split(std::move(arg_packs));
        return split_arg_packs.head.construct(
          [ &constructor
          , &on_success, &on_fail
          , arg_packs = std::move(split_arg_packs.tail)
          , thunks = std::move(thunks)
          ] (auto thunk) mutable {
            multifail_impl(
              constructor
            , on_success, on_fail
            , std::move(arg_packs)
            , tuple_list::combine(std::move(thunks), std::move(thunk))
            );
          }
        , on_fail
        );
      }
    };

    template <
      typename Constructor
    , typename OnSuccess, typename OnFail
    , typename Thunks
    > struct MultifailImpl<
        Constructor
      , OnSuccess, OnFail
      , std::tuple<>
      , Thunks
      > {
      auto operator()(
        Constructor& constructor
      , OnSuccess& on_success, OnFail&
      , std::tuple<>, Thunks packed_thunks
      ) const {
        return forward_tuple(
          [&constructor, &on_success](auto... thunks) {
            return on_success(
              forward(constructor, std::move(thunks)...)
            );
          }
        , std::move(packed_thunks)
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

  template <typename T>
  struct Factory {
    template <typename OnSuccess, typename OnFail, typename ...Args>
    auto operator()(OnSuccess&& on_success, OnFail&&, Args&&... args) const {
      return on_success(
        forward(
          [](auto&&... args) {
            // Note that we explicitly brace construct
            return T{std::forward<decltype(args)>(args)...};
          }
        , std::forward<Args>(args)...
        )
      );
    }
  };

  template <typename T>
  constexpr Factory<T> factory{};

  template <typename T>
  struct Aggregate {
    template <
      typename OnSuccess, typename OnFail
    , typename ...TBuilders
    > auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , TBuilders... t_builders
    ) const {
      // This function iterates through the argument packs, generating a thunk
      // for each member. If it encounters an error, it calls the on_fail
      // callback and returns that function's result.
      return multifail(
        [](auto&&... args) {
          return T{std::forward<decltype(args)>(args)...};
        }
      , on_success
      , on_fail
      , std::move(t_builders)...
      );
    }
  };

  template <typename T>
  constexpr Aggregate<T> aggregate{};
}}

#endif

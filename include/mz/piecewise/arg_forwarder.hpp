#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/make_from_tuple.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mz { namespace piecewise {
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
    auto split_arg_packs = tuple_list::split(std::move(arg_packs));
    return split_arg_packs.unpack_head().construct(
      [ arg_packs = std::move(split_arg_packs.tail)
      , thunks = std::move(thunks)
      , &on_success
      , &on_fail
      ] (auto thunk) {
        multifail(
          std::move(arg_packs)
        , tuple_list::combine(thunk, thunks)
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
  , OnSuccess&& on_success
  , OnFail&&
  ) {
    return forward_tuple(std::move(thunks), on_success);
  }
}}

#endif

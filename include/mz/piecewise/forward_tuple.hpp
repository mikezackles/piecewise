#ifndef UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7
#define UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7

#include <tuple>
#include <type_traits>
#include <utility>

namespace mz { namespace piecewise {
  namespace detail {
    template <
      typename Callback
    , typename ...Args
    , std::size_t ...Indices
    , typename ...ExtraArgs
    > inline auto unpack_tuple(
      Callback& callback
    , std::tuple<Args...> args
    , std::index_sequence<Indices...>
    , ExtraArgs&&... extra_args
    ) {
      return callback(
        std::forward<ExtraArgs>(extra_args)...
      , std::forward<Args>(std::get<Indices>(args))...
      );
    }
  }

  template <
    typename Callback
  , typename ...Args
  , typename ...ExtraArgs
  > inline auto forward_tuple(
    Callback&& callback
  , std::tuple<Args...> args
  , ExtraArgs&&... extra_args
  ) {
    return detail::unpack_tuple(
      callback
    , std::move(args)
    , std::make_index_sequence<sizeof...(Args)>{}
    , std::forward<ExtraArgs>(extra_args)...
    );
  }
}}

#endif

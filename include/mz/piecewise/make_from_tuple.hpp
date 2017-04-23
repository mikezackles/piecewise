#ifndef UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7
#define UUID_3B9EC844_BFEE_4E1F_AA7D_540B1D69E7F7

#include <tuple>
#include <type_traits>
#include <utility>

namespace mz { namespace piecewise {
  namespace detail {
    template <
      typename ...Args
    , std::size_t ...Indices
    , typename Callback
    , typename ...ExtraArgs
    > auto
    unpack_tuple(
      std::tuple<Args...> args
    , std::index_sequence<Indices...>
    , Callback&& callback
    , ExtraArgs&&... extra_args
    ) {
      return callback(
        std::forward<Args>(std::get<Indices>(args))...
      , std::forward<ExtraArgs>(extra_args)...
      );
    }
  }

  template <
    typename ...Args
  , typename Callback
  , typename ...ExtraArgs
  > auto
  forward_tuple(
    std::tuple<Args...> args
  , Callback&& callback
  , ExtraArgs&&... extra_args
  ) {
    return detail::unpack_tuple(
      std::move(args)
    , std::make_index_sequence<sizeof...(Args)>{}
    , std::forward<Callback>(callback)
    , std::forward<ExtraArgs>(extra_args)...
    );
  }
}}

#endif

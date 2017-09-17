#ifndef UUID_B51C0054_47EF_4DA3_9822_00D9EC9458A6
#define UUID_B51C0054_47EF_4DA3_9822_00D9EC9458A6

#include <mz/piecewise/multifail.hpp>

namespace mz { namespace piecewise {
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
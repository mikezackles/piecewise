#ifndef UUID_11DC3752_4553_42AC_BAC5_C9B26D68632C
#define UUID_11DC3752_4553_42AC_BAC5_C9B26D68632C

#include <mz/piecewise/forward_tuple.hpp>

namespace mz { namespace piecewise {
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
}}

#endif
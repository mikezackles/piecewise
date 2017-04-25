#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/forward_tuple.hpp>

namespace mz { namespace piecewise {
  template <typename Callback, typename ...RefTypes>
  class Wrapper {
  public:
    Wrapper(Callback&& callback_, std::tuple<RefTypes...> packed_args_)
      : packed_args{std::move(packed_args_)}
      , callback{callback_}
    {}

    template <typename ...Args>
    auto construct(Args&&... args) {
      return forward_tuple(
        callback
      , std::move(packed_args)
      , std::forward<Args>(args)...
      );
    }

  private:
    std::tuple<RefTypes...> packed_args;
    Callback& callback;
  };

  template <typename Callback, typename ...RefTypes>
  inline auto make_wrapper(
    Callback&& callback
  , std::tuple<RefTypes...> packed_args
  )-> Wrapper<Callback, RefTypes...> {
    return {callback, std::move(packed_args)};
  }

  template <typename Callback, typename ...Args>
  inline auto forward(Callback&& callback, Args&&... args) {
    return make_wrapper(
      callback
    , std::forward_as_tuple(std::forward<Args>(args)...)
    );
  }

  template <typename T>
  struct Construct {
    template <typename ...Args>
    auto operator()(Args&&... args) const {
      return T(std::forward<Args>(args)...);
    }
  };

  template <typename T>
  struct BracedConstruct {
    template <typename ...Args>
    auto operator()(Args&&... args) const {
      return T{std::forward<Args>(args)...};
    }
  };
}}

// We use UUIDs here to avoid name conflicts
// Note that this is just sugar and completely optional!
#ifndef PIECEWISE_NO_MACROS
  #define PIECEWISE_CONSTRUCT(T) \
    [](auto&&... args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D) { \
      return T( \
        std::forward< \
          decltype(args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D) \
        >( \
          args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D \
        )... \
      ); \
    }

  #define PIECEWISE_BRACED_CONSTRUCT(T) \
    [](auto&&... args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D) { \
      return T{ \
        std::forward< \
          decltype(args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D) \
        >( \
          args_A2D4A30C_4719_4075_B8AA_554DF8C9BE9D \
        )... \
      }; \
    }
#endif

#endif

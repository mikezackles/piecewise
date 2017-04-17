#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/make_from_tuple.hpp>

namespace mz { namespace piecewise {
  template <typename Callback, typename ...RefTypes>
  class Wrapper {
  public:
    Wrapper(std::tuple<RefTypes...> packed_args_, Callback callback_)
      : packed_args{std::move(packed_args_)}, callback{std::move(callback_)}
    {}

    auto construct() {
      return forward_tuple(std::move(packed_args), callback);
    }

  private:
    std::tuple<RefTypes...> packed_args;
    Callback callback;
  };

  template <typename Callback, typename ...RefTypes>
  static Wrapper<Callback, RefTypes...> make_wrapper(std::tuple<RefTypes...> packed_args, Callback callback) {
    return {std::move(packed_args), std::move(callback)};
  }

  template <typename Callback, typename ...Args>
  static auto forward(Callback callback, Args&&... args) {
    return make_wrapper(std::forward_as_tuple(std::forward<Args>(args)...), std::move(callback));
  }
}}

#define CONSTRUCT_(T, args) [](auto&&... args) { return T(std::forward<decltype(args)>(args)...); }
#define BRACED_(T, args) [](auto&&... args) { return T{std::forward<decltype(args)>(args)...}; }
#define CONSTRUCT(T) CONSTRUCT_(T, args)
#define BRACED(T) BRACED_(T, args)

#endif

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
  inline auto builder(Callback&& callback, Args&&... args) {
    return make_wrapper(
      callback
    , std::forward_as_tuple(std::forward<Args>(args)...)
    );
  }

  template <typename T>
  auto construct = [](auto... args) {
    return T(std::forward<decltype(args)>(args)...);
  };

  template <typename T>
  auto braced_construct = [](auto... args) {
    return T{std::forward<decltype(args)>(args)...};
  };
}}

#endif

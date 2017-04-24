#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/forward_tuple.hpp>

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
  inline auto make_wrapper(
    std::tuple<RefTypes...> packed_args
  , Callback&& callback
  )-> Wrapper<Callback, RefTypes...> {
    return {std::move(packed_args), std::forward<Callback>(callback)};
  }

  template <typename Callback, typename ...Args>
  inline auto forward(Callback&& callback, Args&&... args) {
    return make_wrapper(
      std::forward_as_tuple(std::forward<Args>(args)...)
    , std::forward<Callback>(callback)
    );
  }
}}

#endif

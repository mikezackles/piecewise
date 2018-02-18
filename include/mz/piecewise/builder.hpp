#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/forward_tuple.hpp>

namespace mz { namespace piecewise {
  template <typename ConstructCallback, typename ...Forwards>
  class Builder {
  public:
    Builder(ConstructCallback callback_, std::tuple<Forwards...> packed_args_)
      : packed_args{std::move(packed_args_)}
      , callback{std::move(callback_)}
    {}

    template <typename ...Args>
    auto construct(Args&&... args) && {
      return forward_tuple(
        callback
      , std::move(packed_args)
      , std::forward<Args>(args)...
      );
    }

  private:
    std::tuple<Forwards...> packed_args;
    ConstructCallback callback;
  };

  template <typename ConstructCallback, typename ...Forwards>
  inline auto make_builder(
    ConstructCallback callback
  , std::tuple<Forwards...> packed_args
  )-> Builder<ConstructCallback, Forwards...> {
    return {std::move(callback), std::move(packed_args)};
  }

  template <typename ConstructCallback, typename ...Args>
  inline auto builder(ConstructCallback callback, Args&&... args) {
    return make_builder(
      std::move(callback)
    , std::forward_as_tuple(std::forward<Args>(args)...)
    );
  }
}}

#endif

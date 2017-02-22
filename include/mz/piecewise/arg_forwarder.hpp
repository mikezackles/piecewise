#ifndef UUID_49150B38_5CFC_48B8_91E5_4A965B99305D
#define UUID_49150B38_5CFC_48B8_91E5_4A965B99305D

#include <mz/piecewise/make_from_tuple.hpp>

namespace mz { namespace piecewise {
  template <typename T>
  class ArgForwarder {
  public:
    template <typename ...RefTypes>
    class Wrapper {
    public:
      Wrapper(std::tuple<RefTypes...> packed_args_)
        : packed_args{std::move(packed_args_)}
      {}

      T construct() {
        return forward_tuple(
          std::move(packed_args)
        , [](auto&&... args) { return T(std::forward<decltype(args)>(args)...); }
        );
      }

      T braced_construct() {
        return forward_tuple(
          std::move(packed_args)
        , [](auto&&... args) { return T{std::forward<decltype(args)>(args)...}; }
        );
      }

    private:
      std::tuple<RefTypes...> packed_args;
    };

    template <typename ...Args>
    static Wrapper<Args&&...> forward(Args&&... args) {
      return {std::forward_as_tuple(std::forward<Args>(args)...)};
    }
  };
}}

#endif

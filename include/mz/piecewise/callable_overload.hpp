#ifndef UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878
#define UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878

#include <type_traits>

namespace mz { namespace piecewise {
#if __cplusplus >= 201703L
  template <typename ...Lambdas>
  struct CallableOverload : Lambdas... {
    using Lambdas::operator()...;
  };
#else
  template <typename Lambda, typename ...Lambdas>
  struct CallableOverload : Lambda, CallableOverload<Lambdas...> {
    using Lambda::operator();
    using CallableOverload<Lambdas...>::operator();
    template <typename ForwardLambda, typename ...ForwardLambdas>
    CallableOverload(ForwardLambda&& lambda, ForwardLambdas&&... lambdas)
      : Lambda{std::forward<ForwardLambda>(lambda)}
      , CallableOverload<Lambdas...>{std::forward<ForwardLambdas>(lambdas)...}
    {}
  };

  template <typename Lambda>
  struct CallableOverload<Lambda> : Lambda {
    using Lambda::operator();
    CallableOverload(Lambda lambda) : Lambda{std::move(lambda)} {}
  };
#endif

  template <typename ...ForwardLambdas>
  auto handler(ForwardLambdas&&... lambdas) {
    return CallableOverload<std::remove_reference_t<ForwardLambdas>...>{
      std::forward<ForwardLambdas>(lambdas)...
    };
  }
}}

#endif

#ifndef UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878
#define UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878

#include <type_traits>

namespace mz { namespace piecewise {
#if __cplusplus >= 201703L
  template <typename ...Lambdas>
  struct LambdaOverload : Lambdas... {
    using Lambdas::operator()...;
  };
#else
  template <typename Lambda, typename ...Lambdas>
  struct LambdaOverload : Lambda, LambdaOverload<Lambdas...> {
    using Lambda::operator();
    using LambdaOverload<Lambdas...>::operator();
    template <typename ForwardLambda, typename ...ForwardLambdas>
    LambdaOverload(ForwardLambda&& lambda, ForwardLambdas&&... lambdas)
      : Lambda{std::forward<ForwardLambda>(lambda)}
      , LambdaOverload<Lambdas...>{std::forward<ForwardLambdas>(lambdas)...}
    {}
  };

  template <typename Lambda>
  struct LambdaOverload<Lambda> : Lambda {
    using Lambda::operator();
    LambdaOverload(Lambda lambda) : Lambda{std::move(lambda)} {}
  };
#endif

  template <typename ...ForwardLambdas>
  auto handler(ForwardLambdas&&... lambdas) {
    return LambdaOverload<std::remove_reference_t<ForwardLambdas>...>{
      std::forward<ForwardLambdas>(lambdas)...
    };
  }
}}

#endif

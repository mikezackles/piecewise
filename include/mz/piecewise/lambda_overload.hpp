#ifndef UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878
#define UUID_AA9FDD71_5F9D_4874_B5E2_888E951DB878

namespace mz { namespace piecewise {
  template <typename Lambda, typename ...Lambdas>
  struct LambdaOverload : Lambda, LambdaOverload<Lambdas...> {
    using Lambda::operator();
    using LambdaOverload<Lambdas...>::operator();
    LambdaOverload(Lambda lambda, Lambdas... lambdas)
      : Lambda{std::move(lambda)}, LambdaOverload<Lambdas...>{std::move(lambdas)...} {}
  };

  template <typename Lambda>
  struct LambdaOverload<Lambda> : Lambda {
    using Lambda::operator();
    LambdaOverload(Lambda lambda) : Lambda{std::move(lambda)} {}
  };

  template <typename ...Lambdas>
  auto make_lambda_overload(Lambdas... lambdas) {
    return LambdaOverload<Lambdas...>{std::move(lambdas)...};
  }
}}

#endif

#include <catch.hpp>
#include <mz/piecewise/arg_forwarder.hpp>

namespace mp = mz::piecewise;

namespace {
  struct A final {
    A(std::string foo_, int thirty_three_)
      : foo{std::move(foo_)}, thirty_three{thirty_three_}
    {}

    std::string const &get_foo() const { return foo; }
    int get_thirty_three() const { return thirty_three; }

  private:
    std::string foo;
    int thirty_three;
  };

  struct B final {
    int forty_two;
    std::string bar;
    int seventy_seven;
  };

  template <typename T, typename U>
  class Aggregate final {
  public:
    template <typename TArgs, typename UArgs>
    Aggregate(TArgs t_args, UArgs u_args)
      : t{t_args.construct()}, u{u_args.construct()}
    {}

    T const &get_t() const { return t; }
    U const &get_u() const { return u; }

  private:
    T t;
    U u;
  };

  //template <typename T, typename U>
  //class FailableAggregate final {
  //public:
  //  template <typename TArgs, typename UArgs, typename OnSuccess, typename OnFail>
  //  static auto create(
  //    bool should_fail
  //  , TArgs t_args, UArgs u_args
  //  , OnSuccess&& on_success, OnFail&& on_fail
  //  ) {
  //    if (should_fail) return on_fail();
  //    return on_success(&internal_create, std::move(t_args), std::move(u_args));
  //  }

  //  T const &get_t() const { return t; }
  //  U const &get_u() const { return u; }

  //private:
  //  template <typename TArgs, typename UArgs>
  //  static FailableAggregate internal_create(TArgs t_args, UArgs u_args) {
  //    return {std::move(t_args), std::move(u_args)};
  //  }

  //  template <typename TArgs, typename UArgs>
  //  FailableAggregate(TArgs t_args, UArgs u_args)
  //    : t{t_args.construct()}, u{u_args.construct()}
  //  {}

  //  T t;
  //  U u;
  //};
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      mp::forward(mp::construct<A>{}, "foo", 33)
    , mp::forward(mp::braced_construct<B>{}, 42, "bar", 77)
    };

    THEN("piecewise construction works") {
      REQUIRE(aggregate.get_t().get_thirty_three() == 33);
      REQUIRE(aggregate.get_t().get_foo() == "foo");
      REQUIRE(aggregate.get_u().forty_two == 42);
      REQUIRE(aggregate.get_u().bar == "bar");
      REQUIRE(aggregate.get_u().seventy_seven == 77);
    }
  }
}

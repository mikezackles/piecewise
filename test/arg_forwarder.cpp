#include <catch.hpp>
#include <mz/piecewise/arg_forwarder.hpp>

namespace mp = mz::piecewise;

namespace {
  struct A final {
    A(std::string foo_, int thirty_three_)
      : foo{std::move(foo_)}, thirty_three{thirty_three_}
    {}

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

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      mp::forward(CONSTRUCT(A), "foo", 33)
    , mp::forward(BRACED(B), 42, "bar", 77)
    };

    THEN("piecewise construction works") {
      REQUIRE(aggregate.t.thirty_three == 33);
      REQUIRE(aggregate.t.foo == "foo");
      REQUIRE(aggregate.u.forty_two == 42);
      REQUIRE(aggregate.u.bar == "bar");
      REQUIRE(aggregate.u.seventy_seven == 77);
    }
  }
}

#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
  struct A {
    template <typename ...Args>
    static auto forward(Args&&... args) {
      return std::forward_as_tuple(std::forward<Args>(args)...);
    }

    template <typename ...Args>
    static A create(std::tuple<Args...>&& args) {
      return mz::piecewise::braced_make_from_tuple<A>(std::move(args));
    }

    A(std::string foo_, int thirty_three_) : foo{std::move(foo_)}, thirty_three{thirty_three_} {}

    std::string foo;
    int thirty_three;
  };

  struct B {
    template <typename ...Args>
    static auto forward(Args&&... args) {
      return std::forward_as_tuple(std::forward<Args>(args)...);
    }

    template <typename ...Args>
    static B create(std::tuple<Args...>&& args) {
      return mz::piecewise::braced_make_from_tuple<B>(std::move(args));
    }

    B(int forty_two_, std::string bar_, int seventy_seven_) : forty_two{forty_two_}, bar{std::move(bar_)}, seventy_seven{seventy_seven_} {}

    int forty_two;
    std::string bar;
    int seventy_seven;
  };

  template <typename T, typename U>
  class Aggregate {
  public:
    // Note that we explicitly force the incoming tuples to be rvalues
    template <typename ...TArgs, typename ...UArgs>
    Aggregate(
      std::piecewise_construct_t
    , std::tuple<TArgs...>&& tArgs
    , std::tuple<UArgs...>&& uArgs
    )
      : t{T::create(std::move(tArgs))}
      , u{U::create(std::move(uArgs))}
    {}

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      std::piecewise_construct
    , A::forward("foo", 33)
    , B::forward(42, "bar", 77)
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

#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
  struct A {
    template <typename ...Args>
    static auto forward(Args&&... args) {
      return std::forward_as_tuple(std::forward<Args>(args)...);
    }

    std::string foo;
    int thirtyThree;
  };

  struct B {
    template <typename ...Args>
    static auto forward(Args&&... args) {
      return std::forward_as_tuple(std::forward<Args>(args)...);
    }

    int fortyTwo;
    std::string bar;
    int seventySeven;
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
      : t{mz::piecewise::braced_make_from_tuple<T>(std::move(tArgs))}
      , u{mz::piecewise::braced_make_from_tuple<U>(std::move(uArgs))}
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
      REQUIRE(aggregate.t.thirtyThree == 33);
      REQUIRE(aggregate.t.foo == "foo");
      REQUIRE(aggregate.u.fortyTwo == 42);
      REQUIRE(aggregate.u.bar == "bar");
      REQUIRE(aggregate.u.seventySeven == 77);
    }
  }
}

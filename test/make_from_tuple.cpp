#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
  struct a {
    std::string foo;
    int thirtyThree;
  };

  struct b {
    int fortyTwo;
    std::string bar;
    int seventySeven;
  };

  template <typename T, typename U>
  class aggregate {
  public:
    // Note that we explicitly force the incoming tuples to be rvalues
    template <typename ...TArgs, typename ...UArgs>
    aggregate(
      std::piecewise_construct_t
    , std::tuple<TArgs...>&& tArgs
    , std::tuple<UArgs...>&& uArgs
    )
      : t{mz::piecewise::make_from_tuple<T>(tArgs)}
      , u{mz::piecewise::make_from_tuple<U>(uArgs)}
    {}

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    aggregate<a, b> aggregate{
      std::piecewise_construct
    , std::forward_as_tuple("foo", 33)
    , std::forward_as_tuple(42, "bar", 77)
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

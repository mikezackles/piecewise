#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
  template <typename T>
  class Factory {
  private:
    template <typename ...RefTypes>
    struct Wrapper {
      std::tuple<RefTypes...> packed_args;
    };

  public:
    template <typename ...Args>
    static Wrapper<Args&&...> wrap(Args&&... args) {
      return {std::forward_as_tuple(std::forward<Args>(args)...)};
    }

    template <typename ...RefTypes>
    static T unwrap(Wrapper<RefTypes...>&& wrapped) {
      return mz::piecewise::forward_tuple(
        std::move(wrapped.packed_args)
      , [](auto&&... args) { return T{Private{}, std::forward<decltype(args)>(args)...}; }
      );
    }

  protected:
    struct Private{};
  };

  struct A : public Factory<A> {
    A(Private, std::string foo_, int thirty_three_)
    : foo{std::move(foo_)}, thirty_three{thirty_three_} {}

    std::string foo;
    int thirty_three;
  };

  struct B : public Factory<B> {
    B(Private, int forty_two_, std::string bar_, int seventy_seven_)
    : forty_two{forty_two_}, bar{std::move(bar_)}, seventy_seven{seventy_seven_} {}

    int forty_two;
    std::string bar;
    int seventy_seven;
  };

  template <typename T, typename U>
  class Aggregate {
  public:
    // Note that we explicitly force the incoming tuples to be rvalues
    template <typename TArgs, typename UArgs>
    Aggregate(
      std::piecewise_construct_t
    , TArgs&& tArgs
    , UArgs&& uArgs
    )
      : t{T::unwrap(std::move(tArgs))}
      , u{U::unwrap(std::move(uArgs))}
    {}

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      std::piecewise_construct
    , A::wrap("foo", 33)
    , B::wrap(42, "bar", 77)
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

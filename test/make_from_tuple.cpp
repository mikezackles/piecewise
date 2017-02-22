#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
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
        return mz::piecewise::forward_tuple(
          std::move(packed_args)
        , [](auto&&... args) { return T(std::forward<decltype(args)>(args)...); }
        );
      }

      T braced_construct() {
        return mz::piecewise::forward_tuple(
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

  struct A final : public ArgForwarder<A> {
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
      : t{t_args.construct()}, u{u_args.braced_construct()}
    {}

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      A::forward("foo", 33)
    , ArgForwarder<B>::forward(42, "bar", 77) // alternate syntax (no inheritance required)
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

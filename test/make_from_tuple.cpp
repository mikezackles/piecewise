#include <catch.hpp>
#include <mz/piecewise/make_from_tuple.hpp>

namespace {
  template <typename T, typename Private>
  class BracedConstructor {
  public:
    template <typename ...Args>
    T operator()(Args&&... args) {
      return T{Private{}, std::forward<Args>(args)...};
    }
  };

  template <typename T, typename Private>
  class RegularConstructor {
  public:
    template <typename ...Args>
    T operator()(Args&&... args) {
      return T(Private{}, std::forward<Args>(args)...);
    }
  };

  template <typename T, typename Private>
  class FactoryFunction {
    template <typename ...Args>
    T operator()(Args&&... args) {
      return T::create(Private{}, std::forward<Args>(args)...);
    }
  };

  template <typename T, template <typename, typename> class Unwrapper>
  class Forwarder {
  protected:
    struct Private{};

  public:
    template <typename ...RefTypes>
    class Wrapper {
    public:
      Wrapper(std::tuple<RefTypes...> packed_args_)
        : packed_args{std::move(packed_args_)}
      {}

      T create() {
        return mz::piecewise::forward_tuple(
          std::move(packed_args)
        , Unwrapper<T, Private>{}
        );
      }

    private:
      std::tuple<RefTypes...> packed_args;
    };

    template <typename ...Args>
    static Wrapper<Args&&...> forward(Args&&... args) {
      return {std::forward_as_tuple(std::forward<Args>(args)...)};
    }

    template <typename ...Args>
    static T create(Args&&... args) {
      return forward(std::forward<Args>(args)...).create();
    }
  };

  struct A : public Forwarder<A, BracedConstructor> {
    A(Private, std::string foo_, int thirty_three_)
      : foo{std::move(foo_)}, thirty_three{thirty_three_}
    {}

    std::string foo;
    int thirty_three;
  };

  struct B : public Forwarder<B, BracedConstructor> {
    B(Private, int forty_two_, std::string bar_, int seventy_seven_)
    : forty_two{forty_two_}, bar{std::move(bar_)}, seventy_seven{seventy_seven_} {}

    int forty_two;
    std::string bar;
    int seventy_seven;
  };

  template <typename T, typename U>
  class Aggregate {
  public:
    template <typename TArgs, typename UArgs>
    Aggregate(TArgs t_args, UArgs u_args)
      : t{t_args.create()}, u{u_args.create()}
    {}

    T t;
    U u;
  };
}

SCENARIO("piecewise construction") {
  GIVEN("an aggregate type constructed with rvalue tuples") {
    Aggregate<A, B> aggregate{
      A::forward("foo", 33)
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

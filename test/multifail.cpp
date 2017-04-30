#include <catch.hpp>
#include <mz/piecewise/arg_forwarder.hpp>
#include <mz/piecewise/lambda_overload.hpp>
#include <mz/piecewise/multifail.hpp>

#include <string>

namespace mp = mz::piecewise;

namespace {
  // A simulates a type that could fail during creation. The private constructor
  // is the final step of construction an object of type A, and it is only
  // called if A's factory function has succeeded. Two error types are used to
  // distinguish separate error conditions. Below there are examples of handling
  // both errors generically and of handling them individually.
  class A final {
  public:
    struct Error1 {
      static constexpr auto description = "This is the first error type";
    };

    struct Error2 {
      static constexpr auto description = "This is the second error type";
    };

    std::string const &get_a_string() const { return a_string; }
    int get_an_int() const { return an_int; }

  private:
    friend struct mp::Factory<A>;
    A(std::string a_string_, int an_int_)
      : a_string{std::move(a_string_)}, an_int{an_int_}
    {}

    std::string a_string;
    int an_int;
  };

  // B needs no explicit factory function to be compatible with
  // `mz::piecewise::factory_forward`. Of course it can also still be
  // constructed normally.
  struct B {
    int int_a;
    int int_b;
  };
}

// Here we specialize mz::piecewise::Factory with the definition of our factory
// function for A. The true logic for construction of A lives here. Error cases
// in this function result in calls to the `on_fail` callback, and successful
// construction results in the forwarding of what we will call a "thunk" to the
// `on_success` callback via `mz::piecewise::forward`. A thunk is basically a
// construction callback paired with a group of perfectly forwarded references
// to arguments.
namespace mz { namespace piecewise {
  template <>
  struct Factory<A> {
    template <typename OnSuccess, typename OnFail>
    auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , bool error1, bool error2, std::string a_string, int an_int
    ) const {
      if (error1) return on_fail(A::Error1{});
      if (error2) return on_fail(A::Error2{});
      return on_success(
        mp::forward(
          [](auto&&... args)-> A {
            return {std::forward<decltype(args)>(args)...};
          }
        , std::move(a_string), an_int
        )
      );
    }
  };
}}

namespace {
  template <typename T, typename U, typename V>
  class Aggregate final {
  public:
    T const &get_t() const { return t; }
    U const &get_u() const { return u; }
    V const &get_v() const { return v; }

  private:
    friend struct mp::Factory<Aggregate<T, U, V>>;
    template <typename TArgs, typename UArgs, typename VArgs>
    Aggregate(TArgs t_args, UArgs u_args, VArgs v_args)
      : t{t_args.construct()}
      , u{u_args.construct()}
      , v{v_args.construct()}
    {}

    T t;
    U u;
    V v;
  };
}

namespace mz { namespace piecewise {
  template <typename T, typename U, typename V>
  struct Factory<Aggregate<T, U, V>> {
    template <
      typename OnSuccess, typename OnFail
    , typename TArgs, typename UArgs, typename VArgs
    > auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , TArgs t_args, UArgs u_args, VArgs v_args
    ) const {
      return multifail(
        [](auto&&... args)-> Aggregate<T, U, V> {
          return {std::forward<decltype(args)>(args)...};
        }
      , on_success
      , on_fail
      , std::move(t_args)
      , std::move(u_args)
      , std::move(v_args)
      );
    }
  };
}}

SCENARIO("multifail") {
  GIVEN("a multifail aggregate") {
    bool success = false;
    bool failure1 = false;
    bool failure2 = false;

    WHEN("the first nested construction fails") {
      mp::factory_forward<Aggregate<A, A, B>>(
        mp::factory_forward<A>(false, true, "abc", 42)
      , mp::factory_forward<A>(false, false, "def", 123)
      , mp::factory_forward<B>(5, 6)
      ).construct(
        [&](auto) { success = true; }
      , mp::make_lambda_overload(
          [&](A::Error1) { failure1 = true; }
        , [&](A::Error2) { failure2 = true; }
        )
      );

      THEN("the failure callback is called") {
        REQUIRE(!success);
        REQUIRE(!failure1);
        REQUIRE(failure2);
      }
    }

    WHEN("the second nested construction fails") {
      mp::factory_forward<Aggregate<A, A, B>>(
        mp::factory_forward<A>(false, false, "abc", 42)
      , mp::factory_forward<A>(true, false, "def", 123)
      , mp::factory_forward<B>(5, 6)
      ).construct(
        [&](auto) { success = true; }
      , mp::make_lambda_overload(
          [&](A::Error1) { failure1 = true; }
        , [&](A::Error2) { failure2 = true; }
        )
      );

      THEN("the failure callback is called") {
        REQUIRE(!success);
        REQUIRE(failure1);
        REQUIRE(!failure2);
      }
    }

    WHEN("construction succeeds") {
      mp::factory_forward<Aggregate<A, A, B>>(
        mp::factory_forward<A>(false, false, "abc", 42)
      , mp::factory_forward<A>(false, false, "def", 123)
      , mp::factory_forward<B>(5, 6)
      ).construct(
        [&](auto thunk) {
          success = true;
          auto res = thunk.construct();
          THEN("the nested types contain the correct values") {
            REQUIRE(res.get_t().get_a_string() == "abc");
            REQUIRE(res.get_t().get_an_int() == 42);
            REQUIRE(res.get_u().get_a_string() == "def");
            REQUIRE(res.get_u().get_an_int() == 123);
            REQUIRE(res.get_v().int_a == 5);
            REQUIRE(res.get_v().int_b == 6);
          }
        }
      , [&](auto) {}
      );

      THEN("the success callback is called") {
        REQUIRE(success);
      }
    }
  }
}

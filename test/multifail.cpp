#include <catch.hpp>
#include <mz/piecewise/arg_forwarder.hpp>
#include <mz/piecewise/lambda_overload.hpp>
#include <mz/piecewise/multifail.hpp>

#include <string>

namespace mp = mz::piecewise;

namespace {
  // `A` simulates a type that could fail during creation.
  class A final {
  public:
    // Two error types are used to distinguish separate error conditions. Below
    // there are examples of handling both errors generically and of handling
    // them individually.
    struct StringEmptyError {
      // It's a good idea to give errors a static description so that generic
      // error handlers can print it.
      static constexpr auto description = "String is empty";
    };

    struct IntNegativeError {
      static constexpr auto description = "Int is negative";
    };

    std::string const &get_a_string() const { return a_string; }
    int get_an_int() const { return an_int; }

  private:
    // Give the factory function access to the constructor.
    friend struct mp::Factory<A>;
    // The private constructor is the final step of construction an object of
    // type `A`, and it is only called if `A`'s factory function has succeeded.
    A(std::string a_string_, int an_int_)
      : a_string{std::move(a_string_)}, an_int{an_int_}
    {}

    std::string a_string;
    int an_int;
  };

  // `B` can be constructed normally, so it needs no explicit factory function
  // to be compatible with `mz::piecewise::factory_forward`.
  struct B {
    int int_a;
    int int_b;
  };
}

namespace mz { namespace piecewise {
  // Here we specialize mz::piecewise::Factory with the definition of our
  // factory function for `A`. The true logic for construction of `A` lives
  // here. Error cases in this function result in calls to the `on_fail`
  // callback, and successful construction results in the forwarding of what we
  // will call a "thunk" to the `on_success` callback via
  // `mz::piecewise::forward`. A thunk is basically a construction callback
  // paired with a group of perfectly forwarded references to arguments.
  template <>
  struct Factory<A> {
    template <typename OnSuccess, typename OnFail>
    auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , std::string a_string, int an_int
    ) const {
      // Validate arguments
      if (a_string.empty()) return on_fail(A::StringEmptyError{});
      if (an_int < 0) return on_fail(A::IntNegativeError{});
      // Now the success callback gets a thunk which creates a *valid* instance
      // of `A`. We can now always assume that every instance of `A` satisfies
      // these preconditions. (This doesn't necessarily hold for a moved-from
      // instance of `A`, so it is up to the programmer to avoid such usage.)
      return on_success(
        // Create a thunk
        mp::forward(
          // This is the actual creation callback. Note that this has access to
          // `A`'s constructor because `A` specified `Factory<A>` as a friend.
          [](auto&&... args)-> A {
            return {std::forward<decltype(args)>(args)...};
          }
          // The arguments to be passed to `A`'s constructor
        , std::move(a_string), an_int
        )
      );
    }
  };
}}

namespace {
  // `Aggregate` demonstrates an aggregate type whose private members can all be
  // injected as template parameters. Since some of its members' can fail to be
  // created, it must also define a factory function by specializing
  // `mz::piecewise::Factory`.
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
  // A factory function for `Aggregate`. Notice that we call
  // `mz::piecewise::multifail`. This function takes care of forwarding
  // arguments to the appropriate members and calling the failure callback on
  // any failure.
  template <typename T, typename U, typename V>
  struct Factory<Aggregate<T, U, V>> {
    template <
      typename OnSuccess, typename OnFail
    , typename TArgs, typename UArgs, typename VArgs
    > auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , TArgs t_args, UArgs u_args, VArgs v_args
    ) const {
      // This function iterates through the argument packs, generating a thunk
      // for each member. If it encounters an error, it calls the on_fail
      // callback and returns that function's result.
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
      // Here we specify all the information necessary to construct an
      // `Aggregate<A, A, B>`.
      mp::factory_forward<Aggregate<A, A, B>>(
        // Here we specify the arguments to construct each of the aggregate's
        // nested types. Notice that in this case, the first call should fail
        // validation.
        mp::factory_forward<A>("abc", -42)
      , mp::factory_forward<A>("def", 123)
      , mp::factory_forward<B>(5, 6)
      )
      // Here we pass one lambda to be invoked if the instance is successfully
      // created and one lambda to be invoked if instantiation fails. In this
      // case, the failure callback will receive an error type as an argument.
      .construct(
        [&](auto) { success = true; }
      , // Here we construct a failure callback out of lambdas that pattern
        // matches based on error type. Notice that order makes no difference.
        mp::make_lambda_overload(
          [&](A::IntNegativeError) { failure2 = true; }
        , [&](A::StringEmptyError) { failure1 = true; }
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
        mp::factory_forward<A>("abc", 42)
      , // Should fail validation
        mp::factory_forward<A>("", 123)
      , mp::factory_forward<B>(5, 6)
      ).construct(
        [&](auto) { success = true; }
      , mp::make_lambda_overload(
          [&](A::StringEmptyError) { failure1 = true; }
        , [&](A::IntNegativeError) { failure2 = true; }
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
        mp::factory_forward<A>("abc", 42)
      , mp::factory_forward<A>("def", 123)
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
      , [&](auto /* e */) {
          // We could print the error generically here:
          // ```
          // std::cout << "Error: " << decltype(e)::description << std::endl;
          // ```
        }
      );

      THEN("the success callback is called") {
        REQUIRE(success);
      }
    }
  }
}

#if defined(_MSC_VER)
  #pragma warning( push )
  #pragma warning( disable : 4244 )
#endif
#include <catch.hpp>
#if defined(_MSC_VER)
  #pragma warning( pop )
#endif
#include <mz/piecewise/builder.hpp>
#include <mz/piecewise/callable_overload.hpp>
#include <mz/piecewise/factory.hpp>
#include <mz/piecewise/helpers.hpp>
#include <mz/piecewise/multifail.hpp>

#include <string>

#if __cplusplus >= 201703L
  #define CONSTEXPR constexpr
#else
  #define CONSTEXPR
#endif

namespace mp = mz::piecewise;

namespace {
  // `A` simulates a type that could fail during creation.
  class A final : public mp::Helpers<A>
  {
  private:
    std::string a_string;
    int an_int;

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
    // Give BuilderHelper permission to call the private constructor and the
    // factory member function.
    friend class mp::Helpers<A>;

    // The true logic for construction of `A` lives here. Error cases in this
    // function result in calls to the `on_fail` callback, and successful
    // construction results in the forwarding of what we will call a "builder"
    // to the `on_success` callback via `mz::piecewise::forward`. A builder is
    // basically a construction callback paired with a group of perfectly
    // forwarded references to arguments.
    static CONSTEXPR auto factory() {
      return [](
        auto constructor
      , auto&& on_success, auto&& on_fail
      , std::string a_string, int an_int
      ) {
        // Validate arguments
        if (a_string.empty()) return on_fail(A::StringEmptyError{});
        if (an_int < 0) return on_fail(A::IntNegativeError{});
        // Now the success callback gets a builder which creates a *valid*
        // instance of `A`. We can now always assume that every instance of `A`
        // satisfies these preconditions. (This doesn't necessarily hold for a
        // moved-from instance of `A`, so it is up to the programmer to avoid
        // such usage.)
        return on_success(
          // Create a builder
          mp::builder(
            // This is the actual creation callback.
            constructor
          , // The arguments to be passed to `A`'s constructor
            std::move(a_string), an_int
          )
        );
      };
    }

  public:
    // The private constructor is the final step of construction an object of
    // type `A`, and it is only called if `A`'s factory function has succeeded.
    A(Private, std::string a_string_, int an_int_)
      : a_string{std::move(a_string_)}, an_int{an_int_}
    {}
  };

  // `B` can be constructed normally, so it needs no explicit factory function
  // to be compatible with `mz::piecewise::factory`.
  struct B {
    int int_a;
    int int_b;
  };

  // `Aggregate` demonstrates an aggregate type whose private members can all be
  // injected as template parameters. If any of these members fail to be
  // created, the failure callback will be called, and the aggregate will not be
  // created.
  template <typename T, typename U, typename V>
  class Aggregate final : public mp::Helpers<Aggregate<T, U, V>> {
  public:
    T const &get_t() const { return t; }
    U const &get_u() const { return u; }
    V const &get_v() const { return v; }
    int get_int() const { return an_int; }

  private:
    // Give the helpers permission to call the private constructor and the
    // factory member function.
    friend class mp::Helpers<Aggregate>;

    static CONSTEXPR auto factory() {
      return [](
        auto constructor
      , auto&& on_success, auto&& on_fail
      , auto t_builder, auto u_builder, auto v_builder
      , int an_int_
      ) {
        return mp::multifail(
          constructor
        , on_success
        , on_fail
        , mp::builders(
            std::move(t_builder), std::move(u_builder), std::move(v_builder)
          )
        , mp::arguments(an_int_)
        );
      };
    }

  public:
    template <typename TBuilder, typename UBuilder, typename VBuilder>
    Aggregate(
      typename mp::Helpers<Aggregate>::Private
    , int an_int_
    , TBuilder t_builder, UBuilder u_builder, VBuilder v_builder
    ) : t{std::move(t_builder).construct()}
      , an_int{an_int_}
      , u{std::move(u_builder).construct()}
      , v{std::move(v_builder).construct()}
    {}

  private:
    T t;
    int an_int;
    U u;
    V v;
  };
}

SCENARIO("multifail aggregate") {
  bool success = false;
  bool failure1 = false;
  bool failure2 = false;

  WHEN("the first nested construction fails") {
    // Here we specify all the information necessary to construct an
    // `Aggregate<A, A, B>`.
    Aggregate<A, A, B>::builder(
      // Here we specify the arguments to construct each of the aggregate's
      // nested types. Notice that in this case, the first call should fail
      // validation.
      A::builder("abc", -42)
    , A::builder("def", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    )
    // Here we pass one lambda to be invoked if the instance is successfully
    // created and one lambda to be invoked if instantiation fails. In this
    // case, the failure callback will receive an error type as an argument.
    .construct(
      [&](auto) { success = true; }
    , // Here we construct a failure callback out of lambdas that pattern
      // matches based on error type. Notice that order makes no difference.
      mp::handler(
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
    Aggregate<A, A, B>::builder(
      A::builder("abc", 42)
    , // Should fail validation
      A::builder("", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    ).construct(
      [&](auto) { success = true; }
    , mp::handler(
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
    Aggregate<A, A, B>::builder(
      A::builder("abc", 42)
    , A::builder("def", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    ).construct(
      [&](auto builder) {
        success = true;
        auto res = std::move(builder).construct();
        THEN("the nested types contain the correct values") {
          REQUIRE(res.get_t().get_a_string() == "abc");
          REQUIRE(res.get_t().get_an_int() == 42);
          REQUIRE(res.get_u().get_a_string() == "def");
          REQUIRE(res.get_u().get_an_int() == 123);
          REQUIRE(res.get_v().int_a == 5);
          REQUIRE(res.get_v().int_b == 6);
          REQUIRE(res.get_int() == 3);
        }
      }
    , [&](auto /* e */) {
        // We could print the error generically here:
        // ```
        // std::cerr << "Error: " << decltype(e)::description << std::endl;
        // ```
      }
    );

    THEN("the success callback is called") {
      REQUIRE(success);
    }
  }

#if __cplusplus >= 201703L
  WHEN("variant") {
    // Doesn't work with clang as of version 5.0.0 due to bug
    // https://bugs.llvm.org//show_bug.cgi?id=33222
    auto variant = Aggregate<A, A, B>::variant<
      A::StringEmptyError
    , A::IntNegativeError
    >(
      A::builder("abc", 42)
    , A::builder("def", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    );
    std::visit(
      mp::handler(
        [](Aggregate<A, A, B> const &result) {
          REQUIRE(result.get_t().get_a_string() == "abc");
          REQUIRE(result.get_t().get_an_int() == 42);
          REQUIRE(result.get_u().get_a_string() == "def");
          REQUIRE(result.get_u().get_an_int() == 123);
          REQUIRE(result.get_v().int_a == 5);
          REQUIRE(result.get_v().int_b == 6);
          REQUIRE(result.get_int() == 3);
        }
      , [](auto) { REQUIRE(false); }
      )
    , variant
    );
  }

  WHEN("invalid variant") {
    // Doesn't work with clang as of version 5.0.0 due to bug
    // https://bugs.llvm.org//show_bug.cgi?id=33222
    auto variant = Aggregate<A, A, B>::variant<
      A::StringEmptyError
    , A::IntNegativeError
    >(
      A::builder("abc", 42)
    , A::builder("def", -123)
    , mp::wrapper<B>(5, 6)
    , 3
    );
    bool failed = false;
    std::visit(
      mp::handler(
        [](Aggregate<A, A, B> const &) {
          REQUIRE(false);
        }
      , [&](auto) { failed = true; }
      )
    , variant
    );
    REQUIRE(failed);
  }

  WHEN("optional") {
    auto optional = Aggregate<A, A, B>::optional(
      A::builder("abc", 42)
    , A::builder("def", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    ).construct(
      [](auto) { REQUIRE(false); }
    );

    REQUIRE(optional);
    REQUIRE(optional->get_t().get_a_string() == "abc");
    REQUIRE(optional->get_t().get_an_int() == 42);
    REQUIRE(optional->get_u().get_a_string() == "def");
    REQUIRE(optional->get_u().get_an_int() == 123);
    REQUIRE(optional->get_v().int_a == 5);
    REQUIRE(optional->get_v().int_b == 6);
    REQUIRE(optional->get_int() == 3);
  }

  WHEN("invalid optional") {
    bool failed = false;
    auto optional = Aggregate<A, A, B>::optional(
      A::builder("", 42)
    , A::builder("def", 123)
    , mp::wrapper<B>(5, 6)
    , 3
    ).construct(
      [&](auto) { failed = true; }
    );

    REQUIRE(failed);
    REQUIRE(!optional);
  }
#endif
}

[![Travis Status](https://travis-ci.org/mikezackles/piecewise.svg?branch=master)](https://travis-ci.org/mikezackles/piecewise)
[![AppVeyor Status](https://ci.appveyor.com/api/projects/status/github/mikezackles/piecewise?svg=true&branch=master)](https://ci.appveyor.com/project/mikezackles/piecewise)

Piecewise is an experimental library for structuring code via compile-time
dependency injection and a pattern matching approach to error handling. A
C++14-capable compiler and standard library are required.

Here we set up an aggregate type composed from two other types, one of which can
fail to be created:
```c++
namespace mp = mz::piecewise;

namespace {
  // `A` simulates a type that could fail during creation.
  class A final : private mp::Constructors<A> {
  public:
    // Two error types are used to distinguish separate error conditions. Below
    // there are examples of handling both errors generically and of handling
    // them individually.
    struct StringEmptyError {
      static constexpr auto description = "String is empty";
    };

    struct IntNegativeError {
      static constexpr auto description = "Int is negative";
    };

    std::string const &get_a_string() const { return a_string; }
    int get_an_int() const { return an_int; }

    // The true logic for construction of `A` lives here. Error cases in this
    // function result in calls to the `on_fail` callback, and successful
    // construction results in the forwarding of what we will call a "builder"
    // to the `on_success` callback via `mz::piecewise::forward`. A builder is
    // basically a construction callback paired with a group of perfectly
    // forwarded references to arguments.
    static CONSTEXPR_LAMBDA auto factory() {
      return [](
        auto&& on_success, auto&& on_fail
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
            // This is the actual creation callback. It just calls `A`'s
            // constructor in the normal way
            constructor()
          , // The arguments to be passed to `A`'s constructor
            std::move(a_string), an_int
          )
        );
      };
    }

  private:
    // Give the `Constructors` helper permission to call the private
    // constructor.
    friend class mp::Constructors<A>;
    // The private constructor is the final step of construction an object of
    // type `A`, and it is only called if `A`'s factory function has succeeded.
    A(std::string a_string_, int an_int_)
      : a_string{std::move(a_string_)}, an_int{an_int_}
    {}

    std::string a_string;
    int an_int;
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
  class Aggregate final {
  public:
    T const &get_t() const { return t; }
    U const &get_u() const { return u; }
    V const &get_v() const { return v; }

  private:
    // This gives piecewise the ability to call the private constructor.
    friend struct mp::Aggregate<Aggregate<T, U, V>>;
    template <typename TBuilder, typename UBuilder, typename VBuilder>
    Aggregate(TBuilder t_builder, UBuilder u_builder, VBuilder v_builder)
      : t{t_builder.construct()}
      , u{u_builder.construct()}
      , v{v_builder.construct()}
    {}

    T t;
    U u;
    V v;
  };
}
```

And here we build it:
```c++
mp::builder(
  mp::multifactory<Aggregate<A, A, B>>
, mp::builder(A::factory(), "abc", 42)
, mp::builder(A::factory(), "def", 123)
, mp::builder(mp::factory<B>, 5, 6)
).construct(
  [](auto builder) {
    Aggregate<A, A, B> result = builder.construct();
    // Use it here!
  }
, [](auto error) {
    std::cerr << "Error: " << decltype(error)::description << std::endl;
  }
);
```

The snippets above come from [here](test/multifail.cpp).

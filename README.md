[![Travis Status](https://travis-ci.org/mikezackles/piecewise.svg?branch=master)](https://travis-ci.org/mikezackles/piecewise)
[![AppVeyor Status](https://ci.appveyor.com/api/projects/status/github/mikezackles/piecewise?svg=true&branch=master)](https://ci.appveyor.com/project/mikezackles/piecewise)

[Coverage Report](https://mikezackles.github.io/piecewise/home/travis/build/mikezackles/piecewise/include/mz/piecewise/index.html)

**Disclaimer**: This project is still experimental.

Overview
--

Piecewise is a small C++14 library for structuring code via compile-time
dependency injection. It favors explicit error handling over exceptions.

This library was born out of a desire to reduce the boilerplate associated with
dependency injection in header-only types that have multiple templated members.
It attempts to do so without sacrificing flexibility.

```c++
Foo::builder(
  // Perfectly forward arguments to the Bar member
  Bar::builder("abc", 42)
, // Perfectly forward arguments to the Baz member
  Baz::builder("xyzzy")
).construct(
  // A success callback. It is only called if both the Bar and the Baz instances
  // were successfully instantiated
  [](auto builder) {
    // We now know we can construct a valid Foo! We use `std::move` to indicate
    // that we are potentially stealing resources
    Foo result = std::move(builder).construct();
    // Use it here!
    std::cout << result.bar().get_string(); // "abc"
    std::cout << result.bar().get_int();    // 42
    std::cout << result.baz().get_string(); // "xyzzy"
  }
, // This is called if initialization fails at *any* point
  [](auto error) {
    std::cerr << "Validation failed: " << decltype(error)::description << std::endl;
  }
);

// Or save it for later as a std::variant
auto saved = Foo::variant<Error1, Error2>(
  Bar::builder("abc", 42)
, Baz::builder("xyzzy")
);

// Or use a std::optional
auto maybe_foo = Foo::optional(
  Bar::builder("abc", 42)
, Baz::builder("xyzzy")
).construct(
  [](auto error) {
    std::cerr << "Validation failed: " << decltype(error)::description << std::endl;
  }
);
if (maybe_foo) {
  maybe_foo->do_foo_things();
}
```

Test Matrix
--

This table attempts to document the configurations that are currently tested by
CI. Assume that compiler versions are the latest offered in the listed
environment. Untested configurations may still work.

| Environment | Compiler | Standard Library | Standard | Build Type | Sanitizers |
| --- | --- | --- | --- | --- | --- |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | GCC | libstdc++ | C++14 | Release | None |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | GCC | libstdc++ | C++14 | Debug | Address/Undefined |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libstdc++ | C++14 | Release | None |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libstdc++ | C++14 | Debug | Address/Undefined |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libc++ | C++14 | Release | None |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libc++ | C++14 | Debug | Address/Undefined |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | GCC | libstdc++ | C++17 | Release | None |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | GCC | libstdc++ | C++17 | Debug | Address/Undefined |
| ~~[Arch Travis](https://github.com/mikkeloscar/arch-travis)~~ | ~~Clang~~ | ~~libstdc++~~ | ~~C++17~~ | ~~Release~~ | ~~None~~ |
| ~~[Arch Travis](https://github.com/mikkeloscar/arch-travis)~~ | ~~Clang~~ | ~~libstdc++~~ | ~~C++17~~ | ~~Debug~~ | ~~Address/Undefined~~ |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libc++ | C++17 | Release | None |
| [Arch Travis](https://github.com/mikkeloscar/arch-travis) | Clang | libc++ | C++17 | Debug | Address/Undefined |
| Travis OS X | Apple Clang | libc++ | C++14 | Release | None |
| AppVeyor Windows | MSVC | MS | C++14 | Release | None |

Note that clang C++17 builds with libstdc++ are disabled due to [this bug](https://bugs.llvm.org//show_bug.cgi?id=33222)

Build
--

Piecewise is header only, so you should be able to just set an appropriate
include path, but if you're a [meson](http://mesonbuild.com) user you
can
[use piecewise as a subproject](http://mesonbuild.com/Wrap-dependency-system-manual.html) (currently
untested). Using meson has the added benefits of both enforcing a single
piecewise instance within your meson-enabled dependencies and allowing you to
run piecewise's tests as part of your build.

If you'd like to build the tests manually:

* [Install meson](http://mesonbuild.com/Getting-meson.html) with pip or your preferred package manager
* Install the [ninja build system](https://ninja-build.org/)
* `meson build` generates a ninja build inside the build directory (use the `CXX` env var to control the compiler)
* `meson configure -C build` will list all the knobs you can tweak
* `meson test -C build` will build and run the test suite

Builders
--

Builders are piecewise's fundamental construct. They're essentially callbacks
paired with references to arguments. When the `construct` member function is
called on a `Builder` instance, the captured references are perfectly forwarded
to the callback, along with any arguments to `construct`.
```c++
// This is a pre-factory builder. Its factory callable has not yet run.
Foo::builder(arg1, arg2)
// This construct call invokes the builder's factory callable
.construct(
  // This callback receives a post-factory builder. Its factory callable has
  // succeeded.
  [](auto builder) {
    // ...
  }
, // This callback gets called if the factory callable fails
  [](auto error) {
    // ...
  }
);
```

Since builders are entirely composed of references, you should treat them as
such!
```c++
void does_not_compile() {}
  auto builder = Foo::builder("the 2 is temporary", 2)
  builder.construct([](auto) {}, [](auto) {}); // ERROR!
}

void compiles() {
  auto builder = Foo::builder("the 2 is temporary", 2)
  std::move(builder).construct([](auto) {}, [](auto) {}); // Bad!
}

void correct() {
  Foo::builder("the 2 is temporary", 2).construct([](auto) {}, [](auto) {});
}
```

Builders are designed so that it is impossible to construct a `Foo` instance
without it being valid. But it *is* possible to return values from the success
and error callbacks, as long as their types match.
```c++
int res = Foo::builder(42).construct(
  [](auto) -> int { return 1; }
, [](auto) -> int { return 2; }
);
```

Here's an example of a retry pattern:
```c++
bool connected = false;
while (!connected) {
  Connection::builder("https://github.com").construct(
    [&connected] (auto) { connected = true; }
  , [] (auto) { std::cerr << "Connection failed!" << std::endl; }
  );
}
```

Helpers
--

Piecewise includes a `Helpers` mix-in to help you design a piecewise-enabled
type. If you want to keep your implementation details private, it requires
friendship with your derived class:

```c++
using mp = mz::piecewise;
class Foo final : private mp::Helpers<Foo> {
  // Give the Helpers mixin access to Foo's factory function
  friend mp::Helpers<Foo>;
public:
  // Use mp::Helpers<Foo>::Private here to hide the constructor
  Foo(Private, ...);
};
```

The above boilerplate will enable `Foo::builder`, `Foo::variant`, and
`Foo::optional` where appropriate.

Factory Function
--

In piecewise, the heavy lifting for construction moves to the static
`Foo::factory` function. It accepts two callbacks, one for success, and one for
failure, along with the other arguments passed to `Foo::builder`.

```c++
private:
  static constexpr auto factory() {
    return [](
      auto constructor
    , auto&& on_success, auto&& on_fail
    , std::string a_string, int an_int
    ) {
      // Validate arguments
      if (a_string.empty()) return on_fail(A::StringEmptyError{});
      if (an_int < 0) return on_fail(A::IntNegativeError{});

      // Now the success callback gets a builder which creates a *valid*
      // instance of `Foo`.
      return on_success(
        // Create a builder
        mp::builder(
          // This is the actual creation callback. It just calls Foo's
          // constructor in the normal way
          constructor
        , // The arguments to be passed to Foo's constructor
          std::move(a_string), an_int
        )
      );
    };
  }
```

Multifail
--

Piecewise also contains a helper function called `multifail`. This important
function handles invoking pre-factory builders in succession until it either
successfully collects all required post-factory builders or the failure callback
has been called. If successful, it invokes the requested constructor. Note that
due to an implementation detail it will pass regular arguments to the
constructor *before* it passes builders. Use this helper if your type contains
nested types that are piecewise-enabled.
```c++
  static constexpr auto factory() {
    return [](
      auto constructor
    , auto&& on_success, auto&& on_fail
    , auto builder1, auto builder2
    , int arg1, std::string arg2
    ) {
      // Error handling specific to this type would happen here
      
      // Handle creation of nested types
      return mp::multifail(
        constructor
      , on_success
      , on_fail
      , mp::builders(
          std::move(builder1), std::move(builder2)
        )
      , mp::arguments(arg1, std::move(arg2))
      );
    };
  }
```

Constructor
--

The private constructor is the final step for constructing an object of type
`Foo`, and it is only called if `Foo`'s factory function has succeeded. Notice
that variadic arguments are *not* supported at this phase of construction.
Again, notice that regular arguments are passed before builders.
```c++
  template <typename Builder1, typename Builder2>
  Foo(int arg1, std::string arg2, Builder1 builder1, Builder2 builder2)
    : nested_type1{std::move(sbuilder1).construct()}
    , nested_type2{std::move(builder2).construct()}
    , an_int{arg1}
    , a_string{std::move(arg2)}
  {}
```

Non-Piecewise Types
--

A compatibility wrapper is provided for passing arguments to nested types that
must coexist with piecewise types. Note that this wrapper only works for types
with public constructors.

```c++
struct C {
  int int_a;
  int int_b;
};

Aggregate<A, B>::builder(
  A::builder("abc", -42)
, B::builder(123)
, mp::wrapper<C>(5, 6) // Create a builder for type C
, 3
)
.construct(
// ...
);
```

Non-piecewise types without a public constructor can still be manually wrapped
by defining a factory and passing it to `mp::builder`:

```c++
// Any callable will do
struct CFactory {
  template <typename OnSuccess, typename OnFail>
  auto operator()(OnSuccess&& on_success, OnFail&&, int arg1, int arg2) const {
    return on_success(
      mp::builder(
        [](int arg1_, int arg2_) {
          return C::non_piecewise_factory(arg1_, arg2_);
        }
      , arg1, arg2
      )
    );
  }
};

Aggregate<A, B>::builder(
  A::builder("abc", -42)
, B::builder(123)
, mp::builder(CFactory{}, 5, 6) // Create a builder for type C
, 3
)
.construct(
// ...
);
```

Example
--

See [here](test/multifail.cpp) for a more complete example.

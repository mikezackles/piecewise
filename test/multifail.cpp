#include <catch.hpp>
#include <mz/piecewise/arg_forwarder.hpp>
#include <mz/piecewise/multifail.hpp>

namespace mp = mz::piecewise;

namespace {
  struct A final {
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
}

namespace mz { namespace piecewise {
  template <>
  struct Factory<A> {
    template <typename OnSuccess, typename OnFail>
    auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , bool should_fail, std::string a_string, int an_int
    ) const {
      if (should_fail) return on_fail();
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
  template <typename T, typename U>
  class Aggregate final {
  public:
    T const &get_t() const { return t; }
    U const &get_u() const { return u; }

  private:
    friend struct mp::Factory<Aggregate<T, U>>;
    template <typename TArgs, typename UArgs>
    Aggregate(TArgs t_args, UArgs u_args)
      : t{t_args.construct()}, u{u_args.construct()}
    {}

    T t;
    U u;
  };
}

namespace mz { namespace piecewise {
  template <typename T, typename U>
  struct Factory<Aggregate<T, U>> {
    template <typename OnSuccess, typename OnFail, typename TArgs, typename UArgs>
    auto operator()(
      OnSuccess&& on_success, OnFail&& on_fail
    , TArgs t_args, UArgs u_args
    ) const {
      return multifail(
        [](auto&&... args)-> Aggregate<T, U> {
          return {std::forward<decltype(args)>(args)...};
        }
      , on_success
      , on_fail
      , std::move(t_args)
      , std::move(u_args)
      );
    }
  };
}}

SCENARIO("multifail") {
  GIVEN("a multifail aggregate") {
    bool success = false;
    bool failure = false;

    WHEN("the first nested construction fails") {
      mp::factory_forward<Aggregate<A, A>>(
        mp::factory_forward<A>(true, "abc", 42)
      , mp::factory_forward<A>(false, "def", 123)
      ).construct(
        [&](auto) { success = true; }
      , [&]() { failure = true; }
      );

      THEN("the failure callback is called") {
        REQUIRE(failure);
      }
    }

    WHEN("the second nested construction fails") {
      mp::factory_forward<Aggregate<A, A>>(
        mp::factory_forward<A>(false, "abc", 42)
      , mp::factory_forward<A>(true, "def", 123)
      ).construct(
        [&](auto) { success = true; }
      , [&]() { failure = true; }
      );

      THEN("the failure callback is called") {
        REQUIRE(failure);
      }
    }

    WHEN("construction succeeds") {
      mp::factory_forward<Aggregate<A, A>>(
        mp::factory_forward<A>(false, "abc", 42)
      , mp::factory_forward<A>(false, "def", 123)
      ).construct(
        [](auto thunk) {
          auto res = thunk.construct();
          THEN("the nested types contain the correct values") {
            REQUIRE(res.get_t().get_a_string() == "abc");
            REQUIRE(res.get_t().get_an_int() == 42);
            REQUIRE(res.get_u().get_a_string() == "def");
            REQUIRE(res.get_u().get_an_int() == 123);
          }
        }
      , []() {
          THEN("the on_fail callback should not execute") {
            REQUIRE(false);
          }
        }
      );
    }
  }
}

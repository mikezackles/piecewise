#include <catch.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mt = mz::piecewise::tuple_list;

SCENARIO("tuple lists") {
  GIVEN("a tuple of forwarded arguments, deconstructed into head and tail") {
    std::string lalala = "lalala";
    int forty_two = 42;
    std::string abc = "abc";

    auto original = std::forward_as_tuple(std::move(lalala), forty_two, abc);
    auto res = mt::split(std::move(original));
    // Reusing moved-from objects is OK here, because under the hood we're just
    // copying references.

    THEN("the head preserves the type of the first entry") {
      REQUIRE((
        std::is_same<decltype(res.head), std::tuple<std::string &&>>::value
      ));
    }

    THEN("the head references the value of the first entry") {
      REQUIRE(&std::get<0>(res.head) == &lalala);
    }

    THEN("the tail preserves the types of the trailing entries") {
      REQUIRE((
        std::is_same<decltype(res.tail), std::tuple<int &, std::string &>>::value
      ));
    }

    THEN("the tail references the values of the trailing entries") {
      REQUIRE(&std::get<0>(res.tail) == &forty_two);
      REQUIRE(&std::get<1>(res.tail) == &abc);
    }

    THEN("recombining head and tail should yield the original list") {
      REQUIRE(mt::combine(std::move(res.head), std::move(res.tail)) == original);
    }
  }
}

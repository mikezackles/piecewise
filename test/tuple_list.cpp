#include <catch.hpp>
#include <mz/piecewise/tuple_list.hpp>

namespace mt = mz::piecewise::tuple_list;

SCENARIO("tuple lists") {
  GIVEN("a tuple, deconstructed into head and tail") {
    auto original = std::make_tuple('a', 'b', 'c');
    auto split_res = mt::split(std::move(original));
    // Reusing moved-from objects is OK here, because under the hood we're just
    // copying references.

    THEN("the head is the first element") {
      REQUIRE(split_res.head == 'a');
    }

    THEN("the tail is the rest of the elements") {
      REQUIRE(split_res.tail == std::make_tuple('b', 'c'));
    }

    THEN("recombining head and tail should put the head after the tail") {
      REQUIRE((
        mt::combine(std::move(split_res.tail), std::move(split_res.head))
        == std::make_tuple('b', 'c', 'a')
      ));
    }
  }
}

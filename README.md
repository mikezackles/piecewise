[![Build Status](https://travis-ci.org/mikezackles/piecewise.svg?branch=master)](https://travis-ci.org/mikezackles/piecewise)

This utility assumes a C++14-capable compiler/standard library.

`make_from_tuple` and `braced_make_from_tuple`
--

These helper functions simplify piecewise construction via perfect forwarding.

Given a generic aggregate type:
```c++
#include <mz/piecewise/make_from_tuple.hpp>

template <typename T, typename U>
class Aggregate {
public:
  // Note that we explicitly force the incoming tuples to be rvalues
  template <typename ...TArgs, typename ...UArgs>
  Aggregate(
    std::piecewise_construct_t
  , std::tuple<TArgs...>&& tArgs
  , std::tuple<UArgs...>&& uArgs
  )
    : t{mz::piecewise::braced_make_from_tuple<T>(std::move(tArgs))}
    , u{mz::piecewise::braced_make_from_tuple<U>(std::move(uArgs))}
  {}

  T t;
  U u;
};
```

an instance can be constructed like this:
```c++
struct A {
  template <typename ...Args>
  static auto forward(Args&&... args) {
    return std::forward_as_tuple(std::forward<Args>(args)...);
  }

  template <typename ...Args>
  static A create(std::tuple<Args...>&& args) {
    return mz::piecewise::braced_make_from_tuple<A>(std::move(args));
  }

  std::string foo;
  int thirty_three;
};

struct B {
  template <typename ...Args>
  static auto forward(Args&&... args) {
    return std::forward_as_tuple(std::forward<Args>(args)...);
  }

  template <typename ...Args>
  static B create(std::tuple<Args...>&& args) {
    return mz::piecewise::braced_make_from_tuple<B>(std::move(args));
  }

  int forty_two;
  std::string bar;
  int seventy_seven;
};

Aggregate<A, B> aggregate{
  std::piecewise_construct
, std::forward_as_tuple("foo", 33)
, std::forward_as_tuple(42, "bar", 77)
};
```

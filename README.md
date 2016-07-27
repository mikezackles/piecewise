[![Build Status](https://travis-ci.org/mikezackles/piecewise.svg?branch=master)](https://travis-ci.org/mikezackles/piecewise)

This utility assumes a C++14-capable compiler/standard library.

makeFromTuple
--

This helper function simplifies piecewise construction via perfect forwarding.

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
    : t{mz::piecewise::make_from_tuple<T>(tArgs)}
    , u{mz::piecewise::make_from_tuple<U>(uArgs)}
  {}

  T t;
  U u;
};
```

an instance can be constructed like this:
```c++
struct A {
  std::string foo;
  int thirtyThree;
};

struct B {
  int fortyTwo;
  std::string bar;
  int seventySeven;
};

Aggregate<A, B> aggregate{
  std::piecewise_construct
, std::forward_as_tuple("foo", 33)
, std::forward_as_tuple(42, "bar", 77)
};
```

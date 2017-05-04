// This tells Catch to provide a main(). Only do this in one cpp file.
#define CATCH_CONFIG_MAIN
#if defined(_MSC_VER)
  #pragma warning( push )
  #pragma warning( disable : 4244 )
#endif
#include <catch.hpp>
#if defined(_MSC_VER)
  #pragma warning( pop )
#endif

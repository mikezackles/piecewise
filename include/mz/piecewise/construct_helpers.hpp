#ifndef UUID_4B0F82B9_ED1D_4D07_94FF_29B87A0BB002
#define UUID_4B0F82B9_ED1D_4D07_94FF_29B87A0BB002

namespace mz { namespace piecewise {
  template <typename T>
  auto construct = [](auto... args) {
    return T(std::forward<decltype(args)>(args)...);
  };

  template <typename T>
  auto braced_construct = [](auto... args) {
    return T{std::forward<decltype(args)>(args)...};
  };
}}

#endif
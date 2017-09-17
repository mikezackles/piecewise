#ifndef UUID_C13860C7_1777_4132_9D59_A26F5BB1858A
#define UUID_C13860C7_1777_4132_9D59_A26F5BB1858A

namespace mz { namespace piecewise {
  template <typename T>
  class Constructors {
  public:
    static auto constructor() {
      return [](auto&&... args) {
        return T(std::forward<decltype(args)>(args)...);
      };
    }

    static auto braced_constructor() {
      return [](auto&&... args) {
        return T{std::forward<decltype(args)>(args)...};
      };
    }
  };
}}

#endif
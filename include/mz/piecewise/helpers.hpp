#ifndef UUID_C13860C7_1777_4132_9D59_A26F5BB1858A
#define UUID_C13860C7_1777_4132_9D59_A26F5BB1858A

#include <mz/piecewise/builder.hpp>

namespace mz { namespace piecewise {
  template <typename T>
  class ConstructorHelper {
  private:
    struct ParenthesizedConstructor {
      template <typename ...Args>
      T operator()(Args&&... args) const {
        return T(std::forward<Args>(args)...);
      }
    };

    struct BracedConstructor {
      template <typename ...Args>
      T operator()(Args&&... args) const {
        return T{std::forward<Args>(args)...};
      }
    };

  public:
    static constexpr auto constructor() {
      return ParenthesizedConstructor{};
    }

    static constexpr auto braced_constructor() {
      return BracedConstructor{};
    }
  };

  template <typename T>
  class BuilderHelper {
  public:
    template <typename ...Args>
    static auto builder(Args&&... args) {
      return piecewise::builder(T::factory(), std::forward<Args>(args)...);
    }
  };
}}

#endif

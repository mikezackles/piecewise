#ifndef UUID_C13860C7_1777_4132_9D59_A26F5BB1858A
#define UUID_C13860C7_1777_4132_9D59_A26F5BB1858A

#include <mz/piecewise/builder.hpp>

#if __cplusplus >= 201703L
  #include <optional>
  #include <variant>
#endif

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

#if __cplusplus >= 201703L
  template <typename T>
  class VariantHelper {
  public:
    template <typename ...ErrorTypes, typename ...Args>
    static std::variant<T, ErrorTypes...> variant(Args&&... args) {
      return piecewise::builder(T::factory(), std::forward<Args>(args)...)
      .construct(
        [](auto builder) {
          return std::variant<T, ErrorTypes...>{std::move(builder).construct()};
        }
      , [](auto error) {
          return std::variant<T, ErrorTypes...>{error};
        }
      );
    }
  };

  template <typename T>
  class OptionalHelper {
  public:
    template <typename ...Args>
    class Optional final {
    private:
      Builder<Args...> mBuilder;
    public:
      explicit Optional(Builder<Args...> builder)
      : mBuilder(std::move(builder))
      {}

      template <typename ErrorCallback>
      std::optional<T> construct(ErrorCallback &&error_callback) && {
        return std::move(mBuilder).construct(
          [](auto builder) {
            return std::optional<T>{std::move(builder).construct()};
          }
        , [&](auto error) {
            error_callback(error);
            return std::optional<T>{std::nullopt};
          }
        );
      }
    };

    template <typename ...Args>
    static auto optional_helper(Builder<Args...> builder) {
      return Optional<Args...>(std::move(builder));
    }

    template <typename ...Args>
    static auto optional(Args&&... args) {
      return optional_helper(
        piecewise::builder(T::factory(), std::forward<Args>(args)...)
      );
    }
  };
#endif
}}

#endif

#ifndef UUID_C13860C7_1777_4132_9D59_A26F5BB1858A
#define UUID_C13860C7_1777_4132_9D59_A26F5BB1858A

#include <mz/piecewise/builder.hpp>

#include <type_traits>

#if __cplusplus >= 201703L
  #include <optional>
  #include <variant>
#endif

namespace mz { namespace piecewise {
  template <typename T>
  class BuilderHelper {
  public:
    template <typename ...Args>
    static auto builder(Args&&... args) {
      auto factory_wrapper = [](auto&&... args_) {
        auto constructor = [](auto&&... args__) {
          return typename T::Implementation(
            typename T::Private{}, std::forward<decltype(args__)>(args__)...
          );
        };
        return T::factory()(
          constructor, std::forward<decltype(args_)>(args_)...
        );
      };
      return piecewise::builder(factory_wrapper, std::forward<Args>(args)...);
    }
  };

#if __cplusplus >= 201703L
  template <typename T>
  class VariantHelper {
  public:
    template <typename ...ErrorTypes, typename ...Args>
    static auto variant(Args&&... args) {
      auto factory_wrapper = [](auto&&... args_) {
        auto constructor = [](auto&&... args__) {
          return std::variant<typename T::Implementation, ErrorTypes...>{
            std::in_place_index<0>
          , typename T::Private{}
          , std::forward<decltype(args__)>(args__)...
          };
        };
        return T::factory()(
          constructor, std::forward<decltype(args_)>(args_)...
        );
      };
      return
        piecewise::builder(factory_wrapper, std::forward<Args>(args)...)
        .construct(
          [](auto builder) {
            return std::move(builder).construct();
          }
        , [](auto error) {
            return std::variant<typename T::Implementation, ErrorTypes...>{
              error
            };
          }
        );
      ;
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
      auto construct(ErrorCallback &&error_callback) && {
        return std::move(mBuilder).construct(
          [](auto builder) {
            return std::move(builder).construct();
          }
        , [&](auto error) {
            error_callback(error);
            return std::optional<typename T::Implementation>{};
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
      auto factory_wrapper = [](auto&&... args_) {
        auto constructor = [](auto&&... args__) {
          return std::make_optional<typename T::Implementation>(
            typename T::Private{}, std::forward<decltype(args__)>(args__)...
          );
        };
        return T::factory()(
          constructor, std::forward<decltype(args_)>(args_)...
        );
      };
      return optional_helper(
        piecewise::builder(factory_wrapper, std::forward<Args>(args)...)
      );
    }
  };
#endif

  template <typename Derived>
  class Helpers
    : public BuilderHelper<Helpers<Derived>>
  #if __cplusplus >= 201703L
    , public VariantHelper<Helpers<Derived>>
    , public OptionalHelper<Helpers<Derived>>
  #endif
  {
    using Implementation = Derived;
    static auto factory() { return Derived::factory(); }

    friend class BuilderHelper<Helpers>;
  #if __cplusplus >= 201703L
    friend class VariantHelper<Helpers>;
    friend class OptionalHelper<Helpers>;
  #endif

  protected:
    struct Private {};
  };
}}

#endif

/**
 *
 *  @file Form.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_VALIDATION_FORM_HPP
#define VIX_VALIDATION_FORM_HPP

#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/validation/Schema.hpp>
#include <vix/validation/ValidationError.hpp>
#include <vix/validation/ValidationErrors.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  namespace detail
  {
    /**
     * @brief Detects whether a type defines `using cleaned_type = ...;`.
     *
     * This is used by `Form<Derived>` to decide the validated output type:
     * - if `Derived::cleaned_type` exists, that becomes the output type
     * - otherwise the output type is `Derived`
     */
    template <typename T, typename = void>
    struct has_cleaned_type : std::false_type
    {
    };

    template <typename T>
    struct has_cleaned_type<T, std::void_t<typename T::cleaned_type>> : std::true_type
    {
    };

    template <typename T>
    inline constexpr bool has_cleaned_type_v = has_cleaned_type<T>::value;

    /**
     * @brief Detects `static bool bind(Derived&, const Input&, ValidationErrors&)`.
     *
     * This is the preferred contract because it allows the binder to
     * attach field-level errors directly.
     */
    template <typename Derived, typename Input, typename = void>
    struct has_bind3 : std::false_type
    {
    };

    template <typename Derived, typename Input>
    struct has_bind3<Derived, Input,
                     std::void_t<decltype(Derived::bind(
                         std::declval<Derived &>(),
                         std::declval<const Input &>(),
                         std::declval<ValidationErrors &>()))>> : std::true_type
    {
    };

    template <typename Derived, typename Input>
    inline constexpr bool has_bind3_v = has_bind3<Derived, Input>::value;

    /**
     * @brief Detects `static bool bind(Derived&, const Input&)`.
     *
     * This is supported as a fallback for very small forms, but it cannot
     * provide rich error reporting by itself. In that mode, `Form` adds a
     * generic "__form__" error on failure.
     */
    template <typename Derived, typename Input, typename = void>
    struct has_bind2 : std::false_type
    {
    };

    template <typename Derived, typename Input>
    struct has_bind2<Derived, Input,
                     std::void_t<decltype(Derived::bind(
                         std::declval<Derived &>(),
                         std::declval<const Input &>()))>> : std::true_type
    {
    };

    /**
     * @brief Detects: static bool set(Derived&, std::string_view, std::string_view).
     *
     * Optional ultra-minimal KV binder for examples and simple forms.
     */
    template <typename Derived, typename = void>
    struct has_kv_set : std::false_type
    {
    };

    template <typename Derived>
    struct has_kv_set<Derived, std::void_t<decltype(Derived::set(
                                   std::declval<Derived &>(),
                                   std::declval<std::string_view>(),
                                   std::declval<std::string_view>()))>> : std::true_type
    {
    };

    template <typename Derived>
    inline constexpr bool has_kv_set_v = has_kv_set<Derived>::value;

    /**
     * @brief Detect KV input type: std::vector<std::pair<std::string_view, std::string_view>>.
     */
    template <typename Input>
    struct is_kv_input : std::false_type
    {
    };

    template <>
    struct is_kv_input<std::vector<std::pair<std::string_view, std::string_view>>> : std::true_type
    {
    };

    template <typename Input>
    inline constexpr bool is_kv_input_v = is_kv_input<std::remove_cv_t<Input>>::value;

    template <typename Derived, typename Input>
    inline constexpr bool has_bind2_v = has_bind2<Derived, Input>::value;

    /**
     * @brief Detects whether `Derived` provides a `clean() const` method.
     *
     * If present, `Form` will return `Derived::cleaned_type` (or a compile-time
     * error if the return type doesn't match `cleaned_type`).
     */
    template <typename Derived, typename = void>
    struct has_clean_method : std::false_type
    {
    };

    template <typename Derived>
    struct has_clean_method<Derived, std::void_t<decltype(std::declval<const Derived &>().clean())>> : std::true_type
    {
    };

    template <typename Derived>
    inline constexpr bool has_clean_method_v = has_clean_method<Derived>::value;

    /**
     * @brief Lazy selection of the cleaned output type.
     *
     * We intentionally avoid `std::conditional_t<cond, typename Derived::cleaned_type, Derived>`
     * because the compiler may try to instantiate both branches (which would break
     * when `cleaned_type` doesn't exist).
     */
    template <typename Derived, bool Has = has_cleaned_type_v<Derived>>
    struct cleaned_type_of_impl
    {
      using type = Derived;
    };

    template <typename Derived>
    struct cleaned_type_of_impl<Derived, true>
    {
      using type = typename Derived::cleaned_type;
    };

    template <typename Derived>
    using cleaned_type_of_t = typename cleaned_type_of_impl<Derived>::type;

    /**
     * @brief Build a generic form-level error.
     *
     * This is used when the binder cannot (or chooses not to) provide
     * detailed errors.
     */
    [[nodiscard]] inline ValidationError make_form_error(
        std::string message = "invalid input",
        ValidationErrorCode code = ValidationErrorCode::Format)
    {
      return ValidationError{
          "__form__",
          code,
          std::move(message)};
    }

  } // namespace detail

  /**
   * @class FormResult
   * @brief Value-or-errors result returned by `Form<Derived>::validate(...)`.
   *
   * Vix aims to feel natural for beginners:
   * @code
   * auto r = Form<MyForm>::validate(input);
   * if (!r) {
   *   // iterate r.errors()
   * }
   * @endcode
   *
   * While still being explicit and composable for advanced users.
   *
   * @tparam T Validated output type (either `Derived` or `Derived::cleaned_type`).
   */
  template <typename T>
  class FormResult
  {
  public:
    /// @brief Construct an empty (invalid) result.
    FormResult() = default;

    /// @brief Construct a success result with a validated value.
    explicit FormResult(T v)
        : value_(std::move(v))
    {
    }

    /// @brief Construct a failure result with validation errors.
    explicit FormResult(ValidationErrors e)
        : errors_(std::move(e))
    {
    }

    /**
     * @brief True if the result contains a value and has no errors.
     *
     * This makes `if (!r)` style code ergonomic.
     */
    [[nodiscard]] explicit operator bool() const
    {
      return value_.has_value() && errors_.size() == 0;
    }

    /**
     * @brief Access the validated value (success path).
     *
     * @warning Only call when `static_cast<bool>(*this)` is true.
     */
    [[nodiscard]] const T &value() const
    {
      return *value_;
    }

    /// @copydoc value() const
    [[nodiscard]] T &value()
    {
      return *value_;
    }

    /**
     * @brief Access the error container (failure path).
     *
     * Even on failure, this is always safe to call.
     */
    [[nodiscard]] const ValidationErrors &errors() const
    {
      return errors_;
    }

    /// @copydoc errors() const
    [[nodiscard]] ValidationErrors &errors()
    {
      return errors_;
    }

  private:
    std::optional<T> value_{};
    ValidationErrors errors_{};
  };

  /**
   * @class Form
   * @brief High-level facade for "bind + validate + clean" workflows.
   *
   * `Form<Derived>` is designed for both:
   * - beginners who want a single entry point to validate raw input
   * - experts who want strict contracts, compile-time checks, and reusable schemas
   *
   * Flow:
   * 1. **Bind** raw input into a `Derived` instance (your form model)
   * 2. **Validate** the instance using its `Schema<Derived>`
   * 3. **Return** either a validated value or structured errors
   *
   * @section form_contract Contract for Derived
   *
   * Required:
   * - `static vix::validation::Schema<Derived> schema();`
   *
   * Binding (choose one):
   * - Preferred:
   *   `static bool bind(Derived &out, const Input &in, ValidationErrors &errors);`
   * - Alternative:
   *   `static bool bind(Derived &out, const Input &in);`
   *
   * Clean output (optional):
   * - If you define `using cleaned_type = X;` you should implement:
   *   `X clean() const;`
   * - Otherwise, the validated output is simply `Derived`.
   *
   * @section form_caching Schema caching
   *
   * `schema()` is constructed once per `Derived` (thread-safe since C++11),
   * then reused for every call. This keeps validation cheap at runtime.
   *
   * @tparam Derived The form/model type.
   */
  template <typename Derived>
  class Form
  {
  public:
    /**
     * @brief Validated output type.
     *
     * - Defaults to `Derived`
     * - If `Derived` defines `using cleaned_type = X;`, output becomes `X`
     */
    using cleaned_type = detail::cleaned_type_of_t<Derived>;

    Form() = delete;

    /**
     * @brief Bind and validate raw input into a cleaned value.
     *
     * @tparam Input Raw input type supported by `Derived::bind(...)`
     * @param in Input payload (kv pairs, JSON-like, request body, etc.)
     * @return FormResult<cleaned_type>
     *
     * @code
     * using Input = std::vector<std::pair<std::string_view, std::string_view>>;
     * auto r = vix::validation::Form<RegisterForm>::validate(input);
     * if (!r) {
     *   for (auto &e : r.errors().all()) { ... }
     * }
     * @endcode
     */
    template <typename Input>
    [[nodiscard]] static FormResult<cleaned_type> validate(const Input &in)
    {
      ValidationErrors errors;
      Derived form{};

      // 1) Bind input -> form
      if constexpr (detail::has_bind3_v<Derived, Input>)
      {
        const bool ok = static_cast<bool>(Derived::bind(form, in, errors));
        if (!ok)
        {
          if (errors.size() == 0)
          {
            errors.add(detail::make_form_error());
          }
          return FormResult<cleaned_type>(std::move(errors));
        }
      }
      else if constexpr (detail::has_bind2_v<Derived, Input>)
      {
        const bool ok = static_cast<bool>(Derived::bind(form, in));
        if (!ok)
        {
          errors.add(detail::make_form_error());
          return FormResult<cleaned_type>(std::move(errors));
        }
      }
      else if constexpr (detail::is_kv_input_v<Input> && detail::has_kv_set_v<Derived>)
      {
        for (const auto &kv : in)
        {
          const bool ok = static_cast<bool>(Derived::set(form, kv.first, kv.second));
          if (!ok)
          {
            errors.add(detail::make_form_error(
                "unknown or invalid field: " + std::string(kv.first),
                ValidationErrorCode::Format));
            return FormResult<cleaned_type>(std::move(errors));
          }
        }
      }
      else
      {
        static_assert(vix::validation::detail::dependent_false_v<Input>,
                      "Form::validate(Input): Derived must implement a compatible bind(). "
                      "Expected: static bool bind(Derived&, const Input&, ValidationErrors&) "
                      "or static bool bind(Derived&, const Input&) "
                      "or (KV input) static bool set(Derived&, std::string_view, std::string_view).");
      }

      // 2) Validate using Schema<Derived>
      ValidationResult vr = schema_ref().validate(form);
      if (!vr.ok())
      {
        errors.merge(vr.errors);
        return FormResult<cleaned_type>(std::move(errors));
      }

      // 3) Produce cleaned output (optional)
      if constexpr (detail::has_clean_method_v<Derived>)
      {
        using CleanRet = vix::validation::detail::remove_cvref_t<
            decltype(std::declval<const Derived &>().clean())>;

        static_assert(std::is_same_v<CleanRet, cleaned_type>,
                      "Form: Derived::clean() must return cleaned_type.");

        return FormResult<cleaned_type>(form.clean());
      }
      else
      {
        static_assert(std::is_same_v<cleaned_type, Derived>,
                      "Form: cleaned_type != Derived but Derived::clean() is missing.");
        return FormResult<cleaned_type>(std::move(form));
      }
    }

    // helper KV input type used by validate_kv
    using kv_pair = std::pair<std::string_view, std::string_view>;
    using kv_list = std::initializer_list<kv_pair>;
    using kv_input = std::vector<kv_pair>;

    [[nodiscard]] static FormResult<cleaned_type> validate_kv(kv_list kv)
    {
      kv_input in;
      in.reserve(kv.size());
      for (const auto &p : kv)
        in.push_back(p);
      return validate(in);
    }

    /**
     * @brief Access the cached schema associated with this form type.
     *
     * This is useful for advanced workflows:
     * - validating an already-built instance
     * - introspection tooling
     * - composing with other schema-based systems
     */
    [[nodiscard]] static const Schema<Derived> &schema()
    {
      return schema_ref();
    }

  private:
    /**
     * @brief Internal accessor for the schema cache.
     *
     * Enforces at compile-time that `Derived::schema()` returns `Schema<Derived>`.
     */
    [[nodiscard]] static const Schema<Derived> &schema_ref()
    {
      using Ret = decltype(Derived::schema());

      static_assert(std::is_same_v<std::remove_cv_t<Ret>, Schema<Derived>>,
                    "Form: Derived must implement: static vix::validation::Schema<Derived> schema();");

      static const Schema<Derived> cached = Derived::schema();
      return cached;
    }
  };

} // namespace vix::validation

#endif // VIX_VALIDATION_FORM_HPP

/**
 *
 *  @file Schema.hpp
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
#ifndef VIX_VALIDATION_SCHEMA_HPP
#define VIX_VALIDATION_SCHEMA_HPP

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/validation/Pipe.hpp>
#include <vix/validation/Rule.hpp>
#include <vix/validation/Rules.hpp>
#include <vix/validation/Validate.hpp>
#include <vix/validation/ValidationErrors.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  namespace detail
  {
    /**
     * @brief Helper for clean static_assert messages inside dependent contexts.
     *
     * Used to force a compile-time error only when a template branch is selected.
     * This keeps error messages precise and beginner-friendly.
     */
    template <typename...>
    inline constexpr bool dependent_false_v = false;

    /**
     * @brief Remove const/volatile and reference qualifiers from a type.
     *
     * Equivalent to `std::remove_cvref_t` (C++20), but kept explicit here
     * for clarity across compilers and for beginners reading the code.
     */
    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    /**
     * @brief Trait: true if Ret is (exactly) ValidationResult (cv/ref ignored).
     */
    template <typename Ret>
    struct is_validation_result : std::false_type
    {
    };

    template <>
    struct is_validation_result<ValidationResult> : std::true_type
    {
    };

    template <typename Ret>
    inline constexpr bool is_validation_result_v =
        is_validation_result<remove_cvref_t<Ret>>::value;

    /**
     * @brief Trait: true if Ret is Validator<FieldT> (cv/ref ignored).
     */
    template <typename Ret, typename FieldT>
    inline constexpr bool is_validator_builder_v =
        std::is_same_v<remove_cvref_t<Ret>, Validator<FieldT>>;

    /**
     * @brief Trait: true if Ret is ParsedValidator<ParsedT> (cv/ref ignored).
     */
    template <typename Ret, typename ParsedT>
    inline constexpr bool is_parsed_builder_v =
        std::is_same_v<remove_cvref_t<Ret>, ParsedValidator<ParsedT>>;

    /**
     * @brief Trait: true if Ret is void (cv/ref ignored).
     *
     * Used by Schema::check to enforce that the "errors-out" overload returns void.
     */
    template <typename Ret, typename T>
    inline constexpr bool is_check_void_v =
        std::is_same_v<remove_cvref_t<Ret>, void>;

  } // namespace detail

  /**
   * @class FieldSpec
   * @brief Fluent rule pack for typed field validations.
   *
   * FieldSpec is a tiny builder that collects `Rule<FieldT>` instances.
   * It keeps call sites short and readable, while still providing explicit,
   * composable validation rules.
   *
   * Beginner-friendly usage:
   * @code
   * .field("email", &User::email,
   *        vix::validation::field<std::string>()
   *          .required()
   *          .email()
   *          .length_max(120))
   * @endcode
   *
   * Expert-friendly usage:
   * - Use FieldSpec to build reusable rule packs shared across schemas.
   * - Combine with `Schema::check(...)` for cross-field constraints.
   *
   * @tparam FieldT The field type (ex: std::string, int, double).
   */
  template <typename FieldT>
  class FieldSpec
  {
  public:
    FieldSpec() = default;

    /**
     * @brief Append a rule to this field spec.
     * @param r Rule to add.
     * @return *this for fluent chaining.
     */
    FieldSpec &rule(Rule<FieldT> r)
    {
      rules_.push_back(std::move(r));
      return *this;
    }

    /**
     * @brief Require a non-empty string.
     * @note Enabled only for std::string.
     */
    FieldSpec &required(std::string message = "field is required")
      requires std::is_same_v<FieldT, std::string>
    {
      return rule(rules::required(std::move(message)));
    }

    /**
     * @brief Enforce minimum string length.
     * @note Enabled only for std::string.
     */
    FieldSpec &length_min(std::size_t n, std::string message = "length is below minimum")
      requires std::is_same_v<FieldT, std::string>
    {
      return rule(rules::length_min(n, std::move(message)));
    }

    /**
     * @brief Enforce maximum string length.
     * @note Enabled only for std::string.
     */
    FieldSpec &length_max(std::size_t n, std::string message = "length is above maximum")
      requires std::is_same_v<FieldT, std::string>
    {
      return rule(rules::length_max(n, std::move(message)));
    }

    /**
     * @brief Validate email format.
     * @note Enabled only for std::string.
     */
    FieldSpec &email(std::string message = "invalid email format")
      requires std::is_same_v<FieldT, std::string>
    {
      return rule(rules::email(std::move(message)));
    }

    /**
     * @brief Validate membership in a set of allowed string values.
     * @note Enabled only for std::string.
     */
    FieldSpec &in_set(std::vector<std::string> allowed, std::string message = "value is not allowed")
      requires std::is_same_v<FieldT, std::string>
    {
      return rule(rules::in_set(std::move(allowed), std::move(message)));
    }

    /**
     * @brief Enforce a minimum numeric value.
     * @note Enabled only for arithmetic types.
     */
    FieldSpec &min(FieldT v, std::string message = "value is below minimum")
      requires std::is_arithmetic_v<FieldT>
    {
      return rule(rules::min<FieldT>(v, std::move(message)));
    }

    /**
     * @brief Enforce a maximum numeric value.
     * @note Enabled only for arithmetic types.
     */
    FieldSpec &max(FieldT v, std::string message = "value is above maximum")
      requires std::is_arithmetic_v<FieldT>
    {
      return rule(rules::max<FieldT>(v, std::move(message)));
    }

    /**
     * @brief Enforce a numeric range [a, b].
     * @note Enabled only for arithmetic types.
     */
    FieldSpec &between(FieldT a, FieldT b, std::string message = "value is out of range")
      requires std::is_arithmetic_v<FieldT>
    {
      return rule(rules::between<FieldT>(a, b, std::move(message)));
    }

    /**
     * @brief Access collected rules (read-only).
     */
    [[nodiscard]] const std::vector<Rule<FieldT>> &rules() const
    {
      return rules_;
    }

  private:
    std::vector<Rule<FieldT>> rules_;
  };

  /**
   * @brief Helper to create a FieldSpec<FieldT>.
   *
   * @tparam FieldT Field type.
   */
  template <typename FieldT>
  [[nodiscard]] inline FieldSpec<FieldT> field()
  {
    return FieldSpec<FieldT>{};
  }

  /**
   * @class ParsedSpec
   * @brief Fluent rule pack for string inputs that must be parsed to ParsedT.
   *
   * ParsedSpec provides a declarative way to define:
   * - how a string input should be parsed into a typed value
   * - which typed rules apply after parsing
   * - which message to report if parsing fails
   *
   * Example:
   * @code
   * .parsed<int>("age", &User::age_text,
   *   vix::validation::parsed<int>()
   *     .between(18, 120)
   *     .parse_message("age must be a number"))
   * @endcode
   *
   * @tparam ParsedT Parsed output type (int, double, etc.)
   */
  template <typename ParsedT>
  class ParsedSpec
  {
  public:
    ParsedSpec() = default;

    /**
     * @brief Append a typed rule applied after parsing succeeds.
     */
    ParsedSpec &rule(Rule<ParsedT> r)
    {
      rules_.push_back(std::move(r));
      return *this;
    }

    /**
     * @brief Enforce a minimum numeric value.
     */
    ParsedSpec &min(ParsedT v, std::string message = "value is below minimum")
      requires std::is_arithmetic_v<ParsedT>
    {
      return rule(rules::min<ParsedT>(v, std::move(message)));
    }

    /**
     * @brief Enforce a maximum numeric value.
     */
    ParsedSpec &max(ParsedT v, std::string message = "value is above maximum")
      requires std::is_arithmetic_v<ParsedT>
    {
      return rule(rules::max<ParsedT>(v, std::move(message)));
    }

    /**
     * @brief Enforce a numeric range [a, b].
     */
    ParsedSpec &between(ParsedT a, ParsedT b, std::string message = "value is out of range")
      requires std::is_arithmetic_v<ParsedT>
    {
      return rule(rules::between<ParsedT>(a, b, std::move(message)));
    }

    /**
     * @brief Message used when parsing fails.
     *
     * This message is attached to the field as a validation error if the input
     * cannot be parsed into ParsedT.
     */
    ParsedSpec &parse_message(std::string msg)
    {
      parse_message_ = std::move(msg);
      return *this;
    }

    /**
     * @brief Access collected typed rules (read-only).
     */
    [[nodiscard]] const std::vector<Rule<ParsedT>> &rules() const
    {
      return rules_;
    }

    /**
     * @brief Access parse-failure message (read-only).
     */
    [[nodiscard]] const std::string &parse_message() const
    {
      return parse_message_;
    }

  private:
    std::vector<Rule<ParsedT>> rules_;
    std::string parse_message_{"invalid value"};
  };

  /**
   * @brief Helper to create a ParsedSpec<ParsedT>.
   *
   * @tparam ParsedT Parsed output type.
   */
  template <typename ParsedT>
  [[nodiscard]] inline ParsedSpec<ParsedT> parsed()
  {
    return ParsedSpec<ParsedT>{};
  }

  /**
   * @class Schema
   * @brief Declarative validator for a type T.
   *
   * Schema is the core building block of Vix validation.
   *
   * It stores a list of checks. Each check can validate:
   * - a typed field (`Schema::field`)
   * - a parsed field (`Schema::parsed`)
   * - the whole object (`Schema::check`) for cross-field constraints
   *
   * The schema itself is cheap to copy and easy to construct,
   * but in typical usage it is cached by higher-level wrappers such as
   * `BaseModel<T>` or `Form<T>`.
   *
   * @tparam T Type being validated.
   */
  template <typename T>
  class Schema
  {
  public:
    /// @brief A validation check that can append errors into `ValidationErrors`.
    using CheckFn = std::function<void(const T &, ValidationErrors &)>;

    Schema() = default;

    /**
     * @brief Register a typed field validation using a callable.
     *
     * Callable signature:
     *   (std::string_view field, const FieldT &value)
     *     -> ValidationResult OR Validator<FieldT>
     *
     * This overload is ideal when you want custom logic per field while keeping
     * a fluent validation style.
     */
    template <typename FieldT, typename F>
    Schema &field(std::string field_name, FieldT T::*member, F &&fn)
    {
      using Fn = detail::remove_cvref_t<F>;

      checks_.push_back(
          [name = std::move(field_name),
           member,
           fn2 = Fn(std::forward<F>(fn))](const T &obj, ValidationErrors &out) mutable
          {
            const FieldT &value = obj.*member;

            using Ret = std::invoke_result_t<Fn &, std::string_view, const FieldT &>;

            if constexpr (detail::is_validation_result_v<Ret>)
            {
              ValidationResult r = fn2(name, value);
              out.merge(r.errors);
            }
            else if constexpr (detail::is_validator_builder_v<Ret, FieldT>)
            {
              ValidationResult r = fn2(name, value).result();
              out.merge(r.errors);
            }
            else
            {
              static_assert(detail::dependent_false_v<FieldT>,
                            "Schema::field: callable must return ValidationResult or Validator<FieldT>.");
            }
          });

      return *this;
    }

    /**
     * @brief Register a typed field validation using a FieldSpec.
     *
     * This overload is the most beginner-friendly:
     * it avoids lambdas while staying explicit.
     */
    template <typename FieldT>
    Schema &field(std::string field_name, FieldT T::*member, FieldSpec<FieldT> spec)
    {
      checks_.push_back(
          [name = std::move(field_name),
           member,
           rules = std::move(spec)](const T &obj, ValidationErrors &out) mutable
          {
            const FieldT &value = obj.*member;
            apply_rules_into<FieldT>(name, value, rules.rules(), out);
          });

      return *this;
    }

    /**
     * @brief Register a parsed field validation using a callable.
     *
     * FieldT must be std::string or std::string_view.
     *
     * Callable signature:
     *   (std::string_view field, std::string_view input)
     *     -> ValidationResult OR ParsedValidator<ParsedT>
     */
    template <typename ParsedT, typename FieldT, typename F>
    Schema &parsed(std::string field_name, FieldT T::*member, F &&fn)
      requires(std::is_same_v<FieldT, std::string> || std::is_same_v<FieldT, std::string_view>)
    {
      using Fn = detail::remove_cvref_t<F>;

      checks_.push_back(
          [name = std::move(field_name),
           member,
           fn2 = Fn(std::forward<F>(fn))](const T &obj, ValidationErrors &out) mutable
          {
            std::string_view input;
            if constexpr (std::is_same_v<FieldT, std::string>)
            {
              input = std::string_view(obj.*member);
            }
            else
            {
              input = (obj.*member);
            }

            using Ret = std::invoke_result_t<Fn &, std::string_view, std::string_view>;

            if constexpr (detail::is_validation_result_v<Ret>)
            {
              ValidationResult r = fn2(name, input);
              out.merge(r.errors);
            }
            else if constexpr (detail::is_parsed_builder_v<Ret, ParsedT>)
            {
              ValidationResult r = fn2(name, input).result();
              out.merge(r.errors);
            }
            else
            {
              static_assert(detail::dependent_false_v<FieldT>,
                            "Schema::parsed: callable must return ValidationResult or ParsedValidator<ParsedT>.");
            }
          });

      return *this;
    }

    /**
     * @brief Register a parsed field validation using a ParsedSpec.
     *
     * FieldT must be std::string or std::string_view.
     */
    template <typename ParsedT, typename FieldT>
    Schema &parsed(std::string field_name, FieldT T::*member, ParsedSpec<ParsedT> spec)
      requires(std::is_same_v<FieldT, std::string> || std::is_same_v<FieldT, std::string_view>)
    {
      checks_.push_back(
          [name = std::move(field_name),
           member,
           rules = std::move(spec)](const T &obj, ValidationErrors &out) mutable
          {
            std::string_view input;
            if constexpr (std::is_same_v<FieldT, std::string>)
            {
              input = std::string_view(obj.*member);
            }
            else
            {
              input = (obj.*member);
            }

            ParsedValidator<ParsedT> v(name, input);
            for (const auto &r : rules.rules())
            {
              v.rule(r);
            }
            (void)v.result_into(out, rules.parse_message());
          });

      return *this;
    }

    /**
     * @brief Register a whole-object check (cross-field / invariants).
     *
     * Callable signature options:
     * - (const T&, ValidationErrors&) -> void
     * - (const T&) -> ValidationResult
     *
     * This is where you express constraints such as:
     * - password confirmation matches
     * - start_date <= end_date
     * - at least one of N fields is present
     */
    template <typename F>
    Schema &check(F &&fn)
    {
      using Fn = detail::remove_cvref_t<F>;

      checks_.push_back(
          [fn2 = Fn(std::forward<F>(fn))](const T &obj, ValidationErrors &out) mutable
          {
            if constexpr (std::is_invocable_v<Fn &, const T &, ValidationErrors &>)
            {
              using Ret = std::invoke_result_t<Fn &, const T &, ValidationErrors &>;
              static_assert(detail::is_check_void_v<Ret, T>,
                            "Schema::check: (const T&, ValidationErrors&) callable must return void.");
              fn2(obj, out);
            }
            else if constexpr (std::is_invocable_v<Fn &, const T &>)
            {
              using Ret = std::invoke_result_t<Fn &, const T &>;
              static_assert(detail::is_validation_result_v<Ret>,
                            "Schema::check: (const T&) callable must return ValidationResult.");
              ValidationResult r = fn2(obj);
              out.merge(r.errors);
            }
            else
            {
              static_assert(detail::dependent_false_v<Fn>,
                            "Schema::check: expected (const T&, ValidationErrors&)->void or (const T&)->ValidationResult.");
            }
          });

      return *this;
    }

    /**
     * @brief Execute all checks and return accumulated errors.
     *
     * This function never throws. It returns a `ValidationResult` which
     * contains all errors produced by the schema.
     */
    [[nodiscard]] ValidationResult validate(const T &obj) const
    {
      ValidationErrors out;

      for (const auto &check : checks_)
      {
        if (check)
        {
          check(obj, out);
        }
      }

      return ValidationResult{std::move(out)};
    }

  private:
    std::vector<CheckFn> checks_;
  };

  /**
   * @brief Helper to create a Schema<T>.
   *
   * This is the recommended entry point:
   * @code
   * static Schema<User> schema() {
   *   return vix::validation::schema<User>()
   *     .field(...)
   *     .check(...);
   * }
   * @endcode
   */
  template <typename T>
  [[nodiscard]] inline Schema<T> schema()
  {
    return Schema<T>{};
  }

} // namespace vix::validation

#endif // VIX_VALIDATION_SCHEMA_HPP

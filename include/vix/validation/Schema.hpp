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
#pragma once

#include <concepts>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/validation/Pipe.hpp>
#include <vix/validation/Validate.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{
  namespace detail
  {
    template <typename...>
    inline constexpr bool dependent_false_v = false;

    template <typename T>
    using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

    template <typename Ret>
    struct is_validation_result : std::false_type
    {
    };
    template <>
    struct is_validation_result<ValidationResult> : std::true_type
    {
    };
    template <typename Ret>
    inline constexpr bool is_validation_result_v = is_validation_result<remove_cvref_t<Ret>>::value;

    template <typename Ret, typename FieldT>
    inline constexpr bool is_validator_builder_v =
        std::is_same_v<remove_cvref_t<Ret>, Validator<FieldT>>;

    template <typename Ret, typename ParsedT>
    inline constexpr bool is_parsed_builder_v =
        std::is_same_v<remove_cvref_t<Ret>, ParsedValidator<ParsedT>>;
  } // namespace detail

  /**
   * @brief Schema<T> validates an object of type T by applying field validators.
   *
   * Pydantic-like goal:
   * - Provide a schema builder that is easy to read.
   * - Keep call-sites clean: field(..., lambda) and parsed(..., lambda)
   * - Allow lambdas to return either:
   *   - ValidationResult (explicit)
   *   - Validator<FieldT> (builder) and Schema calls .result()
   *   - ParsedValidator<ParsedT> (builder) and Schema calls .result()
   *
   * Note:
   * - Do NOT expose std::function in the public API of field/parsed,
   *   otherwise lambda conversions can fail in templates.
   */
  template <typename T>
  class Schema
  {
  public:
    using CheckFn = std::function<void(const T &, ValidationErrors &)>;

    Schema() = default;

    // ------------------------------------------------------------
    // field: typed field validation
    // lambda signature: (std::string_view field, const FieldT& value) -> ValidationResult OR Validator<FieldT>
    // ------------------------------------------------------------
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

    // ------------------------------------------------------------
    // parsed: validate a field stored as string or string_view via parse<T>
    // lambda signature: (std::string_view field, std::string_view input) -> ValidationResult OR ParsedValidator<ParsedT>
    // ------------------------------------------------------------
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
              input = std::string_view(obj.*member);
            else
              input = (obj.*member);

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

    // ------------------------------------------------------------
    // validate: execute all checks
    // ------------------------------------------------------------
    [[nodiscard]] ValidationResult validate(const T &obj) const
    {
      ValidationErrors out;
      for (const auto &check : checks_)
      {
        if (check)
          check(obj, out);
      }
      return ValidationResult{std::move(out)};
    }

  private:
    std::vector<CheckFn> checks_;
  };

  /**
   * @brief Helper to create a Schema<T>.
   */
  template <typename T>
  [[nodiscard]] inline Schema<T> schema()
  {
    return Schema<T>{};
  }

} // namespace vix::validation

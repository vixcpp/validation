/**
 *
 *  @file Validate.hpp
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
#ifndef VIX_VALIDATION_VALIDATE_HPP
#define VIX_VALIDATION_VALIDATE_HPP

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <optional>

#include <vix/validation/Rule.hpp>
#include <vix/validation/Rules.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @brief Fluent validation builder for a single field/value.
   *
   * Example:
   *   auto res = validate("age", age)
   *                .min(18)
   *                .max(120)
   *                .result();
   */
  template <typename T>
  class Validator
  {
  public:
    Validator(std::string_view field, const T &value)
        : field_(field), value_(value)
    {
    }

    // Add a custom rule
    Validator &rule(Rule<T> r)
    {
      rules_.push_back(std::move(r));
      return *this;
    }

    // --------------------------------------------
    // Common helpers (enabled depending on T)
    // --------------------------------------------

    // required for std::string
    Validator &required(std::string message = "field is required")
      requires std::is_same_v<T, std::string>
    {
      return rule(rules::required(std::move(message)));
    }

    // required for std::string_view
    Validator &required_sv(std::string message = "field is required")
      requires std::is_same_v<T, std::string_view>
    {
      return rule(rules::required_sv(std::move(message)));
    }

    // required for std::optional<U>
    template <typename U>
    Validator &required(std::string message = "field is required")
      requires std::is_same_v<T, std::optional<U>>
    {
      // FIX: rules::required(...) returns Rule<std::optional<U>>
      return rule(rules::required(std::move(message)));
    }

    // numeric min/max/between
    Validator &min(T min_value, std::string message = "value is below minimum")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::min<T>(min_value, std::move(message)));
    }

    Validator &max(T max_value, std::string message = "value is above maximum")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::max<T>(max_value, std::move(message)));
    }

    Validator &between(T min_value, T max_value, std::string message = "value is out of range")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::between<T>(min_value, max_value, std::move(message)));
    }

    // string length
    Validator &length_min(std::size_t n, std::string message = "length is below minimum")
      requires std::is_same_v<T, std::string>
    {
      return rule(rules::length_min(n, std::move(message)));
    }

    Validator &length_max(std::size_t n, std::string message = "length is above maximum")
      requires std::is_same_v<T, std::string>
    {
      return rule(rules::length_max(n, std::move(message)));
    }

    // string email
    Validator &email(std::string message = "invalid email format")
      requires std::is_same_v<T, std::string>
    {
      return rule(rules::email(std::move(message)));
    }

    // string in_set
    Validator &in_set(std::vector<std::string> allowed, std::string message = "value is not allowed")
      requires std::is_same_v<T, std::string>
    {
      return rule(rules::in_set(std::move(allowed), std::move(message)));
    }

    // --------------------------------------------
    // Execution
    // --------------------------------------------

    [[nodiscard]] ValidationResult result() const
    {
      return apply_rules<T>(field_, value_, rules_);
    }

  private:
    std::string_view field_;
    const T &value_;
    std::vector<Rule<T>> rules_;
  };

  /**
   * @brief Create a Validator for a given field/value.
   */
  template <typename T>
  [[nodiscard]] inline Validator<T> validate(std::string_view field, const T &value)
  {
    return Validator<T>(field, value);
  }

} // namespace vix::validation

#endif

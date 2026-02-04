/**
 *
 *  @file Rule.hpp
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
#ifndef VIX_VALIDATION_RULE_HPP
#define VIX_VALIDATION_RULE_HPP

#include <functional>
#include <initializer_list>
#include <string_view>
#include <utility>
#include <vector>

#include <vix/validation/ValidationErrors.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @brief Rule<T> represents a single validation rule for a value of type T.
   *
   * A rule is a callable that can push errors into ValidationErrors.
   *
   * Signature:
   *   void(std::string_view field, const T &value, ValidationErrors &out)
   */
  template <typename T>
  using Rule = std::function<void(std::string_view, const T &, ValidationErrors &)>;

  /**
   * @brief Apply rules to a value and append errors into an existing collector.
   *
   * Useful when validating multiple fields/models and accumulating everything
   * into a single ValidationErrors instance.
   */
  template <typename T>
  inline void apply_rules_into(
      std::string_view field,
      const T &value,
      const std::vector<Rule<T>> &rules,
      ValidationErrors &out)
  {
    for (const auto &rule : rules)
    {
      if (rule)
      {
        rule(field, value, out);
      }
    }
  }

  /**
   * @brief Apply a list of rules to a value and return a ValidationResult.
   */
  template <typename T>
  [[nodiscard]] inline ValidationResult apply_rules(
      std::string_view field,
      const T &value,
      const std::vector<Rule<T>> &rules)
  {
    ValidationErrors errors;
    apply_rules_into(field, value, rules, errors);
    return ValidationResult{std::move(errors)};
  }

  /**
   * @brief Convenience overload for initializer_list.
   *
   * Example:
   *   auto res = apply_rules("age", age, { rules::min(18), rules::max(120) });
   */
  template <typename T>
  [[nodiscard]] inline ValidationResult apply_rules(
      std::string_view field,
      const T &value,
      std::initializer_list<Rule<T>> rules)
  {
    ValidationErrors errors;

    for (const auto &rule : rules)
    {
      if (rule)
      {
        rule(field, value, errors);
      }
    }

    return ValidationResult{std::move(errors)};
  }

} // namespace vix::validation

#endif

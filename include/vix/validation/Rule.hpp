#pragma once

#include <functional>
#include <string_view>
#include <type_traits>

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
   *   void(std::string_view field, const T& value, ValidationErrors& out)
   */
  template <typename T>
  using Rule = std::function<void(std::string_view, const T &, ValidationErrors &)>;

  /**
   * @brief Apply a list of rules to a value and collect errors.
   */
  template <typename T>
  [[nodiscard]] inline ValidationResult apply_rules(
      std::string_view field,
      const T &value,
      const std::vector<Rule<T>> &rules)
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

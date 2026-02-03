#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include <vix/conversion/ConversionError.hpp>
#include <vix/conversion/Parse.hpp>

#include <vix/validation/Validate.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @brief Convert a conversion error into a validation error (semantic).
   *
   * Notes:
   * - Validation should not leak low-level parsing details by default.
   * - We map it to ValidationErrorCode::Format.
   * - conversion error code is stored in meta for debugging/observability.
   */
  [[nodiscard]] inline ValidationError
  conversion_error_to_validation(
      std::string_view field,
      const vix::conversion::ConversionError &err,
      std::string message = "invalid value")
  {
    ValidationError ve{
        std::string(field),
        ValidationErrorCode::Format,
        std::move(message)};

    ve.meta["conversion_code"] = std::string(vix::conversion::to_string(err.code));
    if (!err.input.empty())
    {
      ve.meta["input"] = std::string(err.input);
    }
    ve.meta["position"] = std::to_string(err.position);

    return ve;
  }

  /**
   * @brief Validate a scalar value that comes as string input:
   * - parse input -> T using vix::conversion
   * - if parse fails => add a format error
   * - else apply validation rules on T
   *
   * Example:
   *   auto res = validate_parsed<int>("age", input)
   *                .between(18, 120)
   *                .result();
   */
  template <typename T>
  class ParsedValidator
  {
  public:
    ParsedValidator(std::string_view field, std::string_view input)
        : field_(field), input_(input)
    {
    }

    // Add a rule (applies only if parsing succeeds)
    ParsedValidator &rule(Rule<T> r)
    {
      rules_.push_back(std::move(r));
      return *this;
    }

    // Numeric helpers
    ParsedValidator &min(T v, std::string message = "value is below minimum")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::min<T>(v, std::move(message)));
    }

    ParsedValidator &max(T v, std::string message = "value is above maximum")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::max<T>(v, std::move(message)));
    }

    ParsedValidator &between(T a, T b, std::string message = "value is out of range")
      requires std::is_arithmetic_v<T>
    {
      return rule(rules::between<T>(a, b, std::move(message)));
    }

    [[nodiscard]] ValidationResult result(std::string parse_message = "invalid value") const
    {
      ValidationErrors out;

      auto parsed = vix::conversion::parse<T>(input_);
      if (!parsed)
      {
        out.add(conversion_error_to_validation(field_, parsed.error(), std::move(parse_message)));
        return ValidationResult{std::move(out)};
      }

      const T &value = parsed.value();

      for (const auto &r : rules_)
      {
        if (r)
        {
          r(field_, value, out);
        }
      }

      return ValidationResult{std::move(out)};
    }

  private:
    std::string_view field_;
    std::string_view input_;
    std::vector<Rule<T>> rules_;
  };

  /**
   * @brief Factory for ParsedValidator<T>.
   */
  template <typename T>
  [[nodiscard]] inline ParsedValidator<T>
  validate_parsed(std::string_view field, std::string_view input)
  {
    return ParsedValidator<T>(field, input);
  }

} // namespace vix::validation

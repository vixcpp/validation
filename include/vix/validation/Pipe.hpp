/**
 *
 *  @file Pipe.hpp
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
#ifndef VIX_VALIDATION_PIPE_HPP
#define VIX_VALIDATION_PIPE_HPP

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/conversion/ConversionError.hpp>
#include <vix/conversion/Parse.hpp>

#include <vix/validation/Rule.hpp>
#include <vix/validation/Rules.hpp>
#include <vix/validation/ValidationError.hpp>
#include <vix/validation/ValidationErrors.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @brief Convert a conversion error into a semantic validation error.
   *
   * Notes:
   * - Validation should not leak low-level parsing details by default.
   * - We map it to ValidationErrorCode::Format.
   * - The conversion error code and other details are stored in meta
   *   for debugging and observability.
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
    ve.meta["position"] = std::to_string(err.position);

    if (!err.input.empty())
    {
      ve.meta["input"] = std::string(err.input);
    }

    return ve;
  }

  /**
   * @brief Fluent validator for string inputs that must be parsed to T first.
   *
   * Flow:
   * - parse input -> T using vix::conversion
   * - if parse fails => push a Format error
   * - else apply typed rules on T
   *
   * Example:
   *   auto res = validate_parsed<int>("age", input)
   *                .between(18, 120)
   *                .result("age must be a number");
   */
  template <typename T>
  class ParsedValidator
  {
  public:
    ParsedValidator(std::string_view field, std::string_view input)
        : field_(field), input_(input)
    {
    }

    ParsedValidator &rule(Rule<T> r)
    {
      rules_.push_back(std::move(r));
      return *this;
    }

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

    /**
     * @brief Execute validation and append errors into an existing container.
     *
     * @return true if ok (no new errors were added), false otherwise.
     */
    [[nodiscard]] bool result_into(
        ValidationErrors &out,
        std::string parse_message = "invalid value") const
    {
      const std::size_t before = out.size();

      auto parsed = vix::conversion::parse<T>(input_);
      if (!parsed)
      {
        out.add(conversion_error_to_validation(field_, parsed.error(), std::move(parse_message)));
        return false;
      }

      const T &value = parsed.value();

      for (const auto &r : rules_)
      {
        if (r)
        {
          r(field_, value, out);
        }
      }

      return out.size() == before;
    }

    /**
     * @brief Execute validation and return a standalone ValidationResult.
     */
    [[nodiscard]] ValidationResult result(
        std::string parse_message = "invalid value") const
    {
      ValidationErrors out;
      (void)result_into(out, std::move(parse_message));
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

#endif

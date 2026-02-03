/**
 *
 *  @file ValidationError.hpp
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
#ifndef VIX_VALIDATION_VALIDATION_ERROR_HPP
#define VIX_VALIDATION_VALIDATION_ERROR_HPP

#include <string>
#include <string_view>
#include <unordered_map>

namespace vix::validation
{

  /**
   * @brief Validation error codes.
   *
   * These codes represent semantic validation failures,
   * not low-level parsing or conversion errors.
   */
  enum class ValidationErrorCode
  {
    Required,
    Min,
    Max,
    LengthMin,
    LengthMax,
    Between,
    Format,
    InSet,
    Custom
  };

  /**
   * @brief Single validation error.
   *
   * This is a semantic, user-facing error that can be
   * serialized (HTTP 400), logged, or accumulated.
   */
  struct ValidationError
  {
    std::string field; // e.g. "email", "age"
    ValidationErrorCode code;
    std::string message; // human-readable (not localized yet)

    // Optional metadata (min, max, expected, etc.)
    std::unordered_map<std::string, std::string> meta;

    ValidationError() = default;

    ValidationError(
        std::string f,
        ValidationErrorCode c,
        std::string msg)
        : field(std::move(f)), code(c), message(std::move(msg))
    {
    }

    ValidationError(
        std::string f,
        ValidationErrorCode c,
        std::string msg,
        std::unordered_map<std::string, std::string> m)
        : field(std::move(f)), code(c), message(std::move(msg)), meta(std::move(m))
    {
    }
  };

  /**
   * @brief String representation of ValidationErrorCode.
   *
   * Useful for JSON responses and logs.
   */
  [[nodiscard]] inline std::string_view
  to_string(ValidationErrorCode code) noexcept
  {
    switch (code)
    {
    case ValidationErrorCode::Required:
      return "required";
    case ValidationErrorCode::Min:
      return "min";
    case ValidationErrorCode::Max:
      return "max";
    case ValidationErrorCode::LengthMin:
      return "length_min";
    case ValidationErrorCode::LengthMax:
      return "length_max";
    case ValidationErrorCode::Between:
      return "between";
    case ValidationErrorCode::Format:
      return "format";
    case ValidationErrorCode::InSet:
      return "in_set";
    case ValidationErrorCode::Custom:
      return "custom";
    default:
      return "unknown";
    }
  }

} // namespace vix::validation

#endif

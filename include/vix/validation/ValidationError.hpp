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
#include <cstdint>

namespace vix::validation
{

  /**
   * @brief Semantic validation error codes.
   *
   * These codes describe *what* rule failed, not *how* a value was parsed.
   * They are stable and intended to be serialized (JSON, logs, APIs).
   */
  enum class ValidationErrorCode : std::uint8_t
  {
    Required = 0,
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
   * Represents a semantic, user-facing validation failure.
   * Typically used for HTTP 400 responses, form errors, or API diagnostics.
   */
  struct ValidationError
  {
    /// Field name (e.g. "email", "age")
    std::string field;

    /// Semantic error code
    ValidationErrorCode code{ValidationErrorCode::Custom};

    /// Human-readable message (not localized yet)
    std::string message;

    /// Optional metadata (min, max, expected values, etc.)
    std::unordered_map<std::string, std::string> meta;

    ValidationError() = default;

    ValidationError(
        std::string f,
        ValidationErrorCode c,
        std::string msg)
        : field(std::move(f)),
          code(c),
          message(std::move(msg))
    {
    }

    ValidationError(
        std::string f,
        ValidationErrorCode c,
        std::string msg,
        std::unordered_map<std::string, std::string> m)
        : field(std::move(f)),
          code(c),
          message(std::move(msg)),
          meta(std::move(m))
    {
    }
  };

  /**
   * @brief Convert ValidationErrorCode to a stable string identifier.
   *
   * Intended for:
   * - JSON responses
   * - logs
   * - client-side error handling
   *
   * @note The returned strings are stable API identifiers.
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

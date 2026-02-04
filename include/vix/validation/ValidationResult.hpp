/**
 *
 *  @file ValidationResult.hpp
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
#ifndef VIX_VALIDATION_VALIDATION_RESULT_HPP
#define VIX_VALIDATION_VALIDATION_RESULT_HPP

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>

#include <vix/validation/ValidationErrors.hpp>

namespace vix::validation
{

  /**
   * @brief Result of a validation operation.
   *
   * ok() == true means no errors.
   * Designed to be merged, accumulated, and serialized (HTTP 400).
   */
  struct ValidationResult
  {
    ValidationErrors errors;

    ValidationResult() = default;

    explicit ValidationResult(ValidationErrors e)
        : errors(std::move(e))
    {
    }

    [[nodiscard]] bool ok() const noexcept
    {
      return errors.ok();
    }

    [[nodiscard]] bool empty() const noexcept
    {
      return errors.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
      return errors.size();
    }

    [[nodiscard]] explicit operator bool() const noexcept
    {
      return ok();
    }

    void merge(const ValidationResult &other)
    {
      errors.merge(other.errors);
    }

    void merge(ValidationResult &&other)
    {
      errors.merge(std::move(other.errors));
    }

    void add(ValidationError e)
    {
      errors.add(std::move(e));
    }

    void add(
        std::string field,
        ValidationErrorCode code,
        std::string message)
    {
      errors.add(std::move(field), code, std::move(message));
    }

    void add(
        std::string field,
        ValidationErrorCode code,
        std::string message,
        std::unordered_map<std::string, std::string> meta)
    {
      errors.add(
          std::move(field),
          code,
          std::move(message),
          std::move(meta));
    }

    void clear() noexcept
    {
      errors.clear();
    }

    static ValidationResult success()
    {
      return ValidationResult{};
    }

    static ValidationResult failure(ValidationErrors e)
    {
      return ValidationResult{std::move(e)};
    }

    static ValidationResult from_errors(ValidationErrors e)
    {
      return ValidationResult{std::move(e)};
    }
  };

} // namespace vix::validation

#endif

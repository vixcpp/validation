#pragma once

#include <utility>

#include <vix/validation/ValidationErrors.hpp>

namespace vix::validation
{

  /**
   * @brief Result of a validation operation.
   *
   * - ok() == true means no errors.
   * - errors contains all accumulated errors.
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

    void merge(const ValidationResult &other)
    {
      errors.merge(other.errors);
    }

    static ValidationResult success()
    {
      return ValidationResult{};
    }

    static ValidationResult failure(ValidationErrors e)
    {
      return ValidationResult{std::move(e)};
    }
  };

} // namespace vix::validation

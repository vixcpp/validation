/**
 *
 *  @file ValidationErrors.hpp
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
#ifndef VIX_VALIDATION_VALIDATION_ERRORS_HPP
#define VIX_VALIDATION_VALIDATION_ERRORS_HPP

#include <vector>
#include <string>
#include <string_view>

#include <vix/validation/ValidationError.hpp>

namespace vix::validation
{

  /**
   * @brief Collection of validation errors.
   *
   * Used to accumulate multiple errors across fields and rules.
   */
  class ValidationErrors
  {
  public:
    using container_type = std::vector<ValidationError>;
    using iterator = container_type::iterator;
    using const_iterator = container_type::const_iterator;

    ValidationErrors() = default;

    // ------------------------------------------------------------
    // Observers
    // ------------------------------------------------------------

    [[nodiscard]] bool empty() const noexcept
    {
      return errors_.empty();
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
      return errors_.size();
    }

    [[nodiscard]] bool ok() const noexcept
    {
      return errors_.empty();
    }

    [[nodiscard]] const container_type &all() const noexcept
    {
      return errors_;
    }

    // ------------------------------------------------------------
    // Modifiers
    // ------------------------------------------------------------

    void add(ValidationError error)
    {
      errors_.push_back(std::move(error));
    }

    void add(
        std::string field,
        ValidationErrorCode code,
        std::string message)
    {
      errors_.emplace_back(
          std::move(field),
          code,
          std::move(message));
    }

    void add(
        std::string field,
        ValidationErrorCode code,
        std::string message,
        std::unordered_map<std::string, std::string> meta)
    {
      errors_.emplace_back(
          std::move(field),
          code,
          std::move(message),
          std::move(meta));
    }

    void merge(const ValidationErrors &other)
    {
      errors_.insert(
          errors_.end(),
          other.errors_.begin(),
          other.errors_.end());
    }

    void clear()
    {
      errors_.clear();
    }

    // ------------------------------------------------------------
    // Iteration
    // ------------------------------------------------------------

    iterator begin() noexcept { return errors_.begin(); }
    iterator end() noexcept { return errors_.end(); }

    const_iterator begin() const noexcept { return errors_.begin(); }
    const_iterator end() const noexcept { return errors_.end(); }

  private:
    container_type errors_;
  };

} // namespace vix::validation

#endif

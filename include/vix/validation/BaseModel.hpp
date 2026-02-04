/**
 *
 *  @file BaseModel.hpp
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
#ifndef VIX_VALIDATION_BASE_MODEL_HPP
#define VIX_VALIDATION_BASE_MODEL_HPP

#include <type_traits>

#include <vix/validation/Schema.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @class BaseModel
   * @brief CRTP base class for schema-driven model validation.
   *
   * BaseModel provides a lightweight, Pydantic-inspired validation interface
   * for user-defined data models. Validation rules are declared once via a
   * static `schema()` function on the derived type and reused across all
   * validation calls.
   *
   * Design goals:
   * - Zero runtime configuration
   * - Strong typing and compile-time guarantees
   * - Deterministic validation behavior
   * - No repeated schema construction
   *
   * Key properties:
   * - Uses CRTP to bind the schema to the derived type
   * - Exposes both instance-level and static validation APIs
   * - Internally caches the schema (constructed once, thread-safe)
   *
   * Requirements for `Derived`:
   * - Must inherit from `BaseModel<Derived>`
   * - Must implement:
   *   `static vix::validation::Schema<Derived> schema();`
   *
   * Example:
   * @code
   * struct RegisterForm : BaseModel<RegisterForm>
   * {
   *   std::string email;
   *   std::string password;
   *
   *   static Schema<RegisterForm> schema()
   *   {
   *     return schema<RegisterForm>()
   *       .field("email", &RegisterForm::email, ...)
   *       .field("password", &RegisterForm::password, ...);
   *   }
   * };
   *
   * RegisterForm form{"user@example.com", "secret"};
   * auto result = form.validate();
   * if (!result.ok()) {
   *   // handle errors
   * }
   * @endcode
   */
  template <typename Derived>
  class BaseModel
  {
  public:
    /**
     * @brief Validate the current object instance.
     *
     * @return ValidationResult containing all accumulated validation errors.
     */
    [[nodiscard]] ValidationResult validate() const
    {
      return schema_ref().validate(self());
    }

    /**
     * @brief Check whether the current object instance is valid.
     *
     * Convenience wrapper around `validate()`.
     *
     * @return true if no validation errors are produced, false otherwise.
     */
    [[nodiscard]] bool is_valid() const
    {
      return validate().ok();
    }

    /**
     * @brief Validate an arbitrary instance of the derived type.
     *
     * This allows validation without constructing a temporary model instance.
     *
     * @param obj Object to validate.
     * @return ValidationResult containing all accumulated validation errors.
     */
    [[nodiscard]] static ValidationResult validate(const Derived &obj)
    {
      return schema_ref().validate(obj);
    }

    /**
     * @brief Access the cached schema associated with the derived type.
     *
     * The schema is constructed once and reused across all validation calls.
     *
     * @return Reference to the cached Schema<Derived>.
     */
    [[nodiscard]] static const Schema<Derived> &schema()
    {
      return schema_ref();
    }

  private:
    /**
     * @brief Internal accessor for the cached schema.
     *
     * Enforces at compile time that the derived type implements a compatible
     * `schema()` function and guarantees single construction.
     */
    [[nodiscard]] static const Schema<Derived> &schema_ref()
    {
      using Ret = decltype(Derived::schema());

      static_assert(std::is_same_v<std::remove_cv_t<Ret>, Schema<Derived>>,
                    "BaseModel: Derived must implement: "
                    "static vix::validation::Schema<Derived> schema();");

      static const Schema<Derived> cached = Derived::schema();
      return cached;
    }

    /**
     * @brief Cast this object to the derived type.
     */
    [[nodiscard]] const Derived &self() const
    {
      return static_cast<const Derived &>(*this);
    }
  };

} // namespace vix::validation

#endif // VIX_VALIDATION_BASE_MODEL_HPP

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
   * @brief CRTP base class for schema-driven validation.
   *
   * BaseModel binds a validation schema to a user type at compile time.
   * The derived type declares validation rules via a single static `schema()`
   * function. The schema is cached internally and reused across all validation
   * calls (constructed once, thread-safe since C++11).
   *
   * Requirements for Derived:
   * - Must inherit from `BaseModel<Derived>`
   * - Must implement:
   *   `static vix::validation::Schema<Derived> schema();`
   *
   * Key properties:
   * - Instance validation: `obj.validate()`
   * - Static validation: `BaseModel::validate(obj)`
   * - Cached schema access: `BaseModel::schema()`
   *
   * Example:
   * @code
   * struct RegisterForm : vix::validation::BaseModel<RegisterForm>
   * {
   *   std::string email;
   *   std::string password;
   *
   *   static vix::validation::Schema<RegisterForm> schema()
   *   {
   *     return vix::validation::schema<RegisterForm>()
   *       .field("email", &RegisterForm::email,
   *              vix::validation::field<std::string>().required().email().length_max(120))
   *       .field("password", &RegisterForm::password,
   *              vix::validation::field<std::string>().required().length_min(8).length_max(64));
   *   }
   * };
   *
   * RegisterForm f;
   * auto r = f.validate();
   * if (!r.ok()) {
   *   // handle r.errors
   * }
   * @endcode
   */
  template <typename Derived>
  class BaseModel
  {
  public:
    /**
     * @brief Validate this instance using the cached schema.
     *
     * @return ValidationResult containing accumulated errors.
     */
    [[nodiscard]] ValidationResult validate() const
    {
      return schema_ref().validate(self());
    }

    /**
     * @brief Convenience validity check for this instance.
     *
     * @return true if validation produced no errors, false otherwise.
     */
    [[nodiscard]] bool is_valid() const
    {
      return validate().ok();
    }

    /**
     * @brief Validate an arbitrary instance of Derived.
     *
     * @param obj Object to validate.
     * @return ValidationResult containing accumulated errors.
     */
    [[nodiscard]] static ValidationResult validate(const Derived &obj)
    {
      return schema_ref().validate(obj);
    }

    /**
     * @brief Access the cached schema for Derived.
     *
     * @return Reference to the cached Schema<Derived>.
     */
    [[nodiscard]] static const Schema<Derived> &schema()
    {
      return schema_ref();
    }

  private:
    /**
     * @brief Internal schema cache accessor.
     *
     * Enforces at compile time that Derived::schema() returns Schema<Derived>.
     * The returned schema is constructed once and reused.
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

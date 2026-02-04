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
   * @brief Pydantic-like base model using CRTP.
   *
   * Usage:
   *   struct RegisterForm : BaseModel<RegisterForm> {
   *     std::string email;
   *     static Schema<RegisterForm> schema();
   *   };
   *
   *   RegisterForm f{...};
   *   auto res = f.validate();
   */
  template <typename Derived>
  class BaseModel
  {
  public:
    [[nodiscard]] ValidationResult validate() const
    {
      return schema_ref().validate(self());
    }

    [[nodiscard]] bool is_valid() const
    {
      return validate().ok();
    }

    static [[nodiscard]] ValidationResult validate(const Derived &obj)
    {
      return schema_ref().validate(obj);
    }

    static [[nodiscard]] Schema<Derived> schema()
    {
      return schema_ref();
    }

  private:
    static [[nodiscard]] Schema<Derived> schema_ref()
    {
      static_assert(std::is_same_v<decltype(Derived::schema()), Schema<Derived>>,
                    "BaseModel: Derived must implement: static vix::validation::Schema<Derived> schema();");
      return Derived::schema();
    }

    [[nodiscard]] const Derived &self() const
    {
      return static_cast<const Derived &>(*this);
    }
  };

} // namespace vix::validation

#endif

#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <vix/validation/Pipe.hpp>
#include <vix/validation/Validate.hpp>
#include <vix/validation/ValidationResult.hpp>

namespace vix::validation
{

  /**
   * @brief Schema<T> validates an object of type T by applying field validators.
   *
   * It stores a list of "checks" (lambdas) executed against a given object.
   */
  template <typename T>
  class Schema
  {
  public:
    using CheckFn = std::function<void(const T &, ValidationErrors &)>;

    Schema() = default;

    /**
     * @brief Add a typed field validator.
     *
     * Example:
     *   schema.field("age", &User::age, [](auto v){
     *     return validate("age", v).between(18, 120).result();
     *   });
     */
    template <typename FieldT>
    Schema &field(
        std::string field_name,
        FieldT T::*member,
        std::function<ValidationResult(std::string_view, const FieldT &)> validator)
    {
      checks_.push_back(
          [name = std::move(field_name), member, validator = std::move(validator)](const T &obj, ValidationErrors &out)
          {
            const FieldT &value = obj.*member;
            ValidationResult r = validator(name, value);
            out.merge(r.errors);
          });
      return *this;
    }

    /**
     * @brief Convenience: typed validator using Validator<FieldT> builder.
     *
     * Example:
     *   schema.field("email", &User::email, [](auto field, const std::string& v){
     *     return validate(field, v).required().email().result();
     *   });
     */
    template <typename FieldT>
    Schema &field(
        std::string field_name,
        FieldT T::*member,
        std::function<Validator<FieldT>(std::string_view, const FieldT &)> builder)
    {
      return field<FieldT>(
          std::move(field_name),
          member,
          [builder = std::move(builder)](std::string_view f, const FieldT &v)
          {
            return builder(f, v).result();
          });
    }

    /**
     * @brief Add a parsed field validator (input comes as string_view).
     *
     * Useful when your object stores raw input as string or string_view
     * but you want to validate parsed value.
     *
     * Example:
     *   schema.parsed<int>("age", &Form::age_str, [](auto f, auto sv){
     *     return validate_parsed<int>(f, sv).between(18,120).result();
     *   });
     */
    template <typename ParsedT, typename FieldT>
    Schema &parsed(
        std::string field_name,
        FieldT T::*member,
        std::function<ValidationResult(std::string_view, std::string_view)> validator)
      requires(std::is_same_v<FieldT, std::string> || std::is_same_v<FieldT, std::string_view>)
    {
      (void)ParsedT{}; // keeps template intent clear

      checks_.push_back(
          [name = std::move(field_name), member, validator = std::move(validator)](const T &obj, ValidationErrors &out)
          {
            std::string_view input;
            if constexpr (std::is_same_v<FieldT, std::string>)
            {
              input = std::string_view(obj.*member);
            }
            else
            {
              input = (obj.*member);
            }

            ValidationResult r = validator(name, input);
            out.merge(r.errors);
          });

      return *this;
    }

    /**
     * @brief Execute schema validation.
     */
    [[nodiscard]] ValidationResult validate(const T &obj) const
    {
      ValidationErrors out;
      for (const auto &check : checks_)
      {
        if (check)
        {
          check(obj, out);
        }
      }
      return ValidationResult{std::move(out)};
    }

  private:
    std::vector<CheckFn> checks_;
  };

  /**
   * @brief Helper to create a Schema<T>.
   */
  template <typename T>
  [[nodiscard]] inline Schema<T> schema()
  {
    return Schema<T>{};
  }

} // namespace vix::validation

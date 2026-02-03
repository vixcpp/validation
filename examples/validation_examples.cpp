/**
 * Validation examples for Vix.cpp
 *
 * This file demonstrates how to use the validation module:
 * - simple field validation
 * - numeric validation
 * - parsed validation (string -> int)
 * - schema / form validation
 *
 * These examples are intended for:
 * - HTTP controllers
 * - JSON / form validation
 * - CLI argument validation
 */

#include <iostream>
#include <optional>
#include <string>

#include <vix/validation/Schema.hpp>
#include <vix/validation/Validate.hpp>
#include <vix/validation/Pipe.hpp>

using namespace vix::validation;

// ------------------------------------------------------------
// Example 1: simple string validation
// ------------------------------------------------------------
void example_simple_string()
{
  std::string email = "john@example.com";

  auto res = validate("email", email)
                 .required()
                 .email()
                 .length_max(120)
                 .result();

  std::cout << "[example_simple_string] ok=" << res.ok() << "\n";
}

// ------------------------------------------------------------
// Example 2: numeric validation (already typed)
// ------------------------------------------------------------
void example_numeric()
{
  int age = 17;

  auto res = validate("age", age)
                 .min(18, "must be adult")
                 .max(120)
                 .result();

  std::cout << "[example_numeric] ok=" << res.ok() << "\n";
}

// ------------------------------------------------------------
// Example 3: parsed validation (string -> int)
// ------------------------------------------------------------
void example_parsed()
{
  std::string age_input = "25"; // try "abc" or "10"

  auto res = validate_parsed<int>("age", age_input)
                 .between(18, 120)
                 .result("age must be a number");

  std::cout << "[example_parsed] ok=" << res.ok() << "\n";
}

// ------------------------------------------------------------
// Example 4: optional field
// ------------------------------------------------------------
void example_optional()
{
  std::optional<int> score = std::nullopt;

  auto res = validate("score", score)
                 .required("score is required")
                 .result();

  std::cout << "[example_optional] ok=" << res.ok() << "\n";
}

// ------------------------------------------------------------
// Example 5: in_set validation
// ------------------------------------------------------------
void example_in_set()
{
  std::string role = "admin";

  auto res = validate("role", role)
                 .required()
                 .in_set({"admin", "user", "guest"})
                 .result();

  std::cout << "[example_in_set] ok=" << res.ok() << "\n";
}

// ------------------------------------------------------------
// Example 6: schema validation (form / entity)
// ------------------------------------------------------------
struct RegisterForm
{
  std::string email;
  std::string password;
  std::string age; // raw input
};

void example_schema()
{
  auto schema = vix::validation::schema<RegisterForm>()
                    .field("email", &RegisterForm::email,
                           [](auto f, const std::string &v)
                           {
                             return validate(f, v)
                                 .required()
                                 .email()
                                 .length_max(120)
                                 .result();
                           })
                    .field("password", &RegisterForm::password,
                           [](auto f, const std::string &v)
                           {
                             return validate(f, v)
                                 .required()
                                 .length_min(8)
                                 .length_max(64)
                                 .result();
                           })
                    .parsed<int>("age", &RegisterForm::age,
                                 [](auto f, std::string_view sv)
                                 {
                                   return validate_parsed<int>(f, sv)
                                       .between(18, 120)
                                       .result("age must be a number");
                                 });

  RegisterForm form{
      "bad-email",
      "123",
      "abc"};

  auto res = schema.validate(form);

  std::cout << "[example_schema] ok=" << res.ok() << "\n";
  std::cout << "errors=" << res.errors.size() << "\n";

  for (const auto &e : res.errors.all())
  {
    std::cout
        << " - field=" << e.field
        << " code=" << to_string(e.code)
        << " message=" << e.message
        << "\n";
  }
}

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main()
{
  example_simple_string();
  example_numeric();
  example_parsed();
  example_optional();
  example_in_set();
  example_schema();

  return 0;
}

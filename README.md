# Vix Validation

The **validation module** provides a modern, declarative, and type-safe way to
validate data in C++.

It is designed for:
- backend APIs
- forms and user input
- configuration files
- domain models
- both beginners and advanced C++ developers

This module favors **clarity**, **composability**, and **zero hidden magic**.

---

## Philosophy

Validation in Vix is built around a few strong principles:

- **Declarative**: describe what is valid, not how to check it
- **Type-safe**: errors are caught at compile time when possible
- **Composable**: rules, schemas, and forms can be reused and combined
- **Explicit**: no exceptions, no implicit conversions
- **Beginner-friendly** but **expert-ready**

You can start with simple string validation and grow toward full form binding
and cleaned output without changing your mental model.

---

## Core Concepts

The module is structured around five main building blocks:

| Concept | Purpose |
|------|-------|
| `Validator<T>` | Fluent validation of a single value |
| `Schema<T>` | Declarative validation rules for a struct |
| `Form<T>` | Bind raw input + validate + produce output |
| `BaseModel<T>` | CRTP helper for schema-driven models |
| `ValidationResult / ValidationErrors` | Structured error reporting |

---

## 1. Validating a Single Value

Use `validate(field, value)` for quick checks.

```cpp
#include <vix/validation/Validate.hpp>

auto r = vix::validation::validate("email", email)
           .required()
           .email()
           .length_max(120)
           .result();

if (!r.ok())
{
  // handle r.errors
}
```

Examples:
- `examples/simple_string.cpp`
- `examples/validate_string.cpp`
- `examples/validate_enum.cpp`

---

## 2. Schema: Declarative Validation for Structs

### FieldSpec (beginner-friendly)

```cpp
struct User
{
  std::string email;
  std::string password;

  static vix::validation::Schema<User> schema()
  {
    return vix::validation::schema<User>()
      .field("email", &User::email,
             vix::validation::field<std::string>()
               .required()
               .email()
               .length_max(120))
      .field("password", &User::password,
             vix::validation::field<std::string>()
               .required()
               .length_min(8)
               .length_max(64));
  }
};
```

Examples:
- `examples/schema_fieldspec_basic.cpp`
- `examples/schema.cpp`

---

### Lambda-based (expert-friendly)

```cpp
.field("email", &User::email,
  [](std::string_view f, const std::string &v)
  {
    return vix::validation::validate(f, v)
      .required()
      .email();
  })
```

Examples:
- `examples/schema_lambda_builder.cpp`

---

### Cross-field validation

```cpp
.check([](const User &u, ValidationErrors &errors)
{
  if (u.password != u.confirm)
  {
    errors.add("confirm",
               ValidationErrorCode::Custom,
               "passwords do not match");
  }
});
```

Examples:
- `examples/schema_cross_field_check.cpp`

---

## 3. Parsed Validation (string to typed)

Examples:
- `examples/parsed.cpp`
- `examples/form_parsed_age.cpp`
- `examples/validate_parsed_int.cpp`

---

## 4. Form: Bind + Validate + Output

Examples:
- `examples/form_kv_basic.cpp`
- `examples/form_cleaned_output.cpp`
- `examples/form_bind2_generic_error.cpp`

---

## 5. BaseModel: Schema-driven Models (CRTP)

Examples:
- `examples/basemodel_basic.cpp`
- `examples/basemodel_static_validate.cpp`
- `examples/basemodel_cross_field_check.cpp`

---

## Error Model

Errors are structured, not strings.

ValidationError fields:
- field
- code
- message

Codes:
- Required
- Min, Max
- LengthMin, LengthMax
- Between
- Format
- InSet
- Custom

---

## Tests

Unit tests live in `tests/`.

Run:
```bash
ctest
```

---

## License

MIT
Part of the Vix.cpp project.

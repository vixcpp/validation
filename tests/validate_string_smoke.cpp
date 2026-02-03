#include <cassert>
#include <iostream>
#include <string>

#include <vix/validation/Validate.hpp>
#include <vix/validation/ValidationError.hpp>

using vix::validation::validate;
using vix::validation::ValidationErrorCode;

int main()
{
  // -------------------------
  // required should fail
  // -------------------------
  {
    std::string email = "";
    auto res = validate("email", email)
                   .required("email is required")
                   .result();

    assert(!res.ok());
    assert(res.errors.size() == 1);
    assert(res.errors.all()[0].field == "email");
    assert(res.errors.all()[0].code == ValidationErrorCode::Required);
  }

  // -------------------------
  // email format should fail
  // -------------------------
  {
    std::string email = "not-an-email";
    auto res = validate("email", email)
                   .required()
                   .email("invalid email")
                   .result();

    assert(!res.ok());
    // required passes, email fails
    assert(res.errors.size() == 1);
    assert(res.errors.all()[0].field == "email");
    assert(res.errors.all()[0].code == ValidationErrorCode::Format);
  }

  // -------------------------
  // length_max should fail
  // -------------------------
  {
    std::string long_email(200, 'a');
    long_email += "@x.com";

    auto res = validate("email", long_email)
                   .length_max(64, "too long")
                   .result();

    assert(!res.ok());
    assert(res.errors.size() == 1);
    assert(res.errors.all()[0].code == ValidationErrorCode::LengthMax);
  }

  // -------------------------
  // multiple errors on same field
  // -------------------------
  {
    std::string bad = ""; // triggers required
    auto res = validate("email", bad)
                   .required("required")
                   .email("invalid email")     // will also fail (because empty triggers format)
                   .length_min(5, "too short") // will also fail
                   .result();

    assert(!res.ok());
    // required + email + length_min
    assert(res.errors.size() == 3);
    for (const auto &e : res.errors.all())
    {
      assert(e.field == "email");
    }
  }

  std::cout << "[validation] validate string smoke tests passed\n";
  return 0;
}

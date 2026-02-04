#include <cassert>
#include <string>
#include <vix/validation/Pipe.hpp>

using namespace vix::validation;

int main()
{
  std::string input = "25";

  auto res = validate_parsed<int>("age", input)
                 .between(18, 120)
                 .result("age must be a number");

  assert(res.ok());
  assert(res.errors.size() == 0);
  return 0;
}

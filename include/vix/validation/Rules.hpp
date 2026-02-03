/**
 *
 *  @file Rules.hpp
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
#ifndef VIX_VALIDATION_RULES_HPP
#define VIX_VALIDATION_RULES_HPP

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <vix/validation/Rule.hpp>
#include <vix/validation/ValidationError.hpp>

namespace vix::validation::rules
{

  namespace detail
  {

    [[nodiscard]] inline std::unordered_map<std::string, std::string>
    meta_kv(std::initializer_list<std::pair<std::string, std::string>> items)
    {
      std::unordered_map<std::string, std::string> m;
      m.reserve(items.size());
      for (const auto &p : items)
      {
        m.emplace(p.first, p.second);
      }
      return m;
    }

    template <typename T>
    [[nodiscard]] inline std::string to_string_value(const T &v)
    {
      if constexpr (std::is_same_v<T, std::string>)
      {
        return v;
      }
      else if constexpr (std::is_same_v<T, std::string_view>)
      {
        return std::string(v);
      }
      else if constexpr (std::is_arithmetic_v<T>)
      {
        return std::to_string(v);
      }
      else
      {
        return "<value>";
      }
    }

  } // namespace detail

  // ------------------------------------------------------------
  // required
  // ------------------------------------------------------------

  /**
   * @brief Required rule for std::string / std::string_view.
   */
  [[nodiscard]] inline Rule<std::string>
  required(std::string message = "field is required")
  {
    return [msg = std::move(message)](std::string_view field, const std::string &value, ValidationErrors &out)
    {
      if (value.empty())
      {
        out.add(std::string(field), ValidationErrorCode::Required, msg);
      }
    };
  }

  [[nodiscard]] inline Rule<std::string_view>
  required_sv(std::string message = "field is required")
  {
    return [msg = std::move(message)](std::string_view field, std::string_view value, ValidationErrors &out)
    {
      if (value.empty())
      {
        out.add(std::string(field), ValidationErrorCode::Required, msg);
      }
    };
  }

  /**
   * @brief Required rule for std::optional<T>.
   */
  template <typename T>
  [[nodiscard]] inline Rule<std::optional<T>>
  required(std::string message = "field is required")
  {
    return [msg = std::move(message)](std::string_view field, const std::optional<T> &value, ValidationErrors &out)
    {
      if (!value.has_value())
      {
        out.add(std::string(field), ValidationErrorCode::Required, msg);
      }
    };
  }

  // ------------------------------------------------------------
  // min / max / between (numbers)
  // ------------------------------------------------------------

  template <typename T>
  [[nodiscard]] inline Rule<T>
  min(T min_value, std::string message = "value is below minimum")
  {
    static_assert(std::is_arithmetic_v<T>, "rules::min<T>: T must be arithmetic");

    return [min_value, msg = std::move(message)](std::string_view field, const T &value, ValidationErrors &out)
    {
      if (value < min_value)
      {
        out.add(
            std::string(field),
            ValidationErrorCode::Min,
            msg,
            detail::meta_kv({{"min", detail::to_string_value(min_value)},
                             {"got", detail::to_string_value(value)}}));
      }
    };
  }

  template <typename T>
  [[nodiscard]] inline Rule<T>
  max(T max_value, std::string message = "value is above maximum")
  {
    static_assert(std::is_arithmetic_v<T>, "rules::max<T>: T must be arithmetic");

    return [max_value, msg = std::move(message)](std::string_view field, const T &value, ValidationErrors &out)
    {
      if (value > max_value)
      {
        out.add(
            std::string(field),
            ValidationErrorCode::Max,
            msg,
            detail::meta_kv({{"max", detail::to_string_value(max_value)},
                             {"got", detail::to_string_value(value)}}));
      }
    };
  }

  template <typename T>
  [[nodiscard]] inline Rule<T>
  between(T min_value, T max_value, std::string message = "value is out of range")
  {
    static_assert(std::is_arithmetic_v<T>, "rules::between<T>: T must be arithmetic");

    return [min_value, max_value, msg = std::move(message)](std::string_view field, const T &value, ValidationErrors &out)
    {
      if (value < min_value || value > max_value)
      {
        out.add(
            std::string(field),
            ValidationErrorCode::Between,
            msg,
            detail::meta_kv({{"min", detail::to_string_value(min_value)},
                             {"max", detail::to_string_value(max_value)},
                             {"got", detail::to_string_value(value)}}));
      }
    };
  }

  // ------------------------------------------------------------
  // length rules (string)
  // ------------------------------------------------------------

  [[nodiscard]] inline Rule<std::string>
  length_min(std::size_t n, std::string message = "length is below minimum")
  {
    return [n, msg = std::move(message)](std::string_view field, const std::string &value, ValidationErrors &out)
    {
      if (value.size() < n)
      {
        out.add(
            std::string(field),
            ValidationErrorCode::LengthMin,
            msg,
            detail::meta_kv({{"min", std::to_string(n)},
                             {"got", std::to_string(value.size())}}));
      }
    };
  }

  [[nodiscard]] inline Rule<std::string>
  length_max(std::size_t n, std::string message = "length is above maximum")
  {
    return [n, msg = std::move(message)](std::string_view field, const std::string &value, ValidationErrors &out)
    {
      if (value.size() > n)
      {
        out.add(
            std::string(field),
            ValidationErrorCode::LengthMax,
            msg,
            detail::meta_kv({{"max", std::to_string(n)},
                             {"got", std::to_string(value.size())}}));
      }
    };
  }

  // ------------------------------------------------------------
  // in_set (string)
  // ------------------------------------------------------------

  [[nodiscard]] inline Rule<std::string>
  in_set(std::vector<std::string> allowed, std::string message = "value is not allowed")
  {
    return [allowed = std::move(allowed), msg = std::move(message)](std::string_view field, const std::string &value, ValidationErrors &out)
    {
      const auto it = std::find(allowed.begin(), allowed.end(), value);
      if (it == allowed.end())
      {
        out.add(
            std::string(field),
            ValidationErrorCode::InSet,
            msg,
            detail::meta_kv({{"got", value}}));
      }
    };
  }

  // ------------------------------------------------------------
  // format helpers (simple email)
  // ------------------------------------------------------------

  /**
   * @brief Very lightweight email format check.
   *
   * Not RFC-complete. Intended as a basic input guard:
   * - contains exactly one '@'
   * - at least one char before '@'
   * - at least one '.' after '@'
   */
  [[nodiscard]] inline Rule<std::string>
  email(std::string message = "invalid email format")
  {
    return [msg = std::move(message)](std::string_view field, const std::string &value, ValidationErrors &out)
    {
      const auto at = value.find('@');
      if (at == std::string::npos || at == 0)
      {
        out.add(std::string(field), ValidationErrorCode::Format, msg);
        return;
      }

      const auto dot = value.find('.', at + 1);
      if (dot == std::string::npos || dot == at + 1 || dot == value.size() - 1)
      {
        out.add(std::string(field), ValidationErrorCode::Format, msg);
        return;
      }
    };
  }

} // namespace vix::validation::rules

#endif

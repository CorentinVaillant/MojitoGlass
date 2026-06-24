#pragma once

// std includes :
#include <array>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
//fmt includes
#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include "ansi_char.hpp"

namespace mjt {

//========== LOG/DEBUG ==========//
#ifndef VERBOSITY
#define VERBOSITY INFO
#endif

// -- Log Levels --
enum LogLvl {
  CRITICAL = 50,
  FATAL    = CRITICAL,
  ERROR    = 40,
  WARNING  = 30,
  WARN     = WARNING,
  INFO     = 20,
  DEBUG    = 10,
  NOTSET   = 0,
};

constexpr bool THROW_ON_MACRO_ERR = true;

#ifndef VERBOSITY
#define LOG(level, ...)                                                        \
  do {                                                                         \
  } while (0)
#else
#define LOG(level, ...)                                                        \
  do {                                                                         \
    if (VERBOSITY <= level) {                                                  \
      fmt::print(                                                              \
        "[{}LOG {}] {}::{} ",                                                  \
        ansi_code::BBLU,                                                       \
        ansi_code::reset,                                                      \
        __func__,                                                              \
        __LINE__);                                                             \
      fmt::println(__VA_ARGS__);                                               \
    }                                                                          \
  } while (0)
#endif

#define LOGOK(...)                                                             \
  do {                                                                         \
    fmt::print(                                                                \
      "[{}OK  {}] {}::{} ",                                                    \
      ansi_code::BGRN,                                                         \
      ansi_code::reset,                                                        \
      __func__,                                                                \
      __LINE__);                                                               \
    fmt::println(__VA_ARGS__);                                                 \
  } while (0)

#define LOGWARN(...)                                                           \
  do {                                                                         \
    if (VERBOSITY <= LogLvl::WARNING)                                          \
      fmt::print(                                                              \
        "[{}WARN{}] {}::{} ",                                                  \
        ansi_code::BYEL,                                                       \
        ansi_code::reset,                                                      \
        __func__,                                                              \
        __LINE__);                                                             \
    fmt::println(__VA_ARGS__);                                                 \
  } while (0)

#define LOGERR(...)                                                            \
  {                                                                            \
    fmt::print(                                                                \
      "[{}ERR {}] {}::{} ",                                                    \
      ansi_code::BRED,                                                         \
      ansi_code::reset,                                                        \
      __func__,                                                                \
      __LINE__);                                                               \
    fmt::println(__VA_ARGS__);                                                 \
    if (THROW_ON_MACRO_ERR)                                                    \
      throw std::runtime_error("Log err");                                     \
  }

#define ASSERT_ERR(expr, ...)                                                  \
  if (!(expr))                                                                 \
  LOGERR(__VA_ARGS__)

#ifndef NDEBUG
#define ON_DEBUG(x) x
#else
#define ON_DEBUG(x)
#endif

//========== Class Decl Helper  ==========//

#define NO_COPY(CLASS_NAME)                                                    \
  CLASS_NAME(const CLASS_NAME &)            = delete;                          \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;

//========== Types ==========//

template <typename T, typename E> class Result {
  union {
    T val;
    E unex;
  };
  bool has_value;

  // Constructor
  struct OkTag {};
  struct ErrTag {};

  Result(OkTag, T &&v) : val(std::forward<T>(v)), has_value(true) {}
  Result(ErrTag, E &&e) : unex(std::forward<E>(e)), has_value(false) {}

  auto destroy() noexcept {
    if (has_value)
      val.~T();
    else
      unex.~E();
  }

public:
  static auto ok(T &&value) { return Result(OkTag{}, std::move(value)); }
  static auto err(E error) { return Result(ErrTag{}, std::move(error)); }
  ~Result() { destroy(); }

  Result(const Result &o) : has_value(o.has_value) {
    if (has_value)
      new (&val) T(o.val);
    else
      new (&unex) E(o.unex);
  }

  Result(Result &&o) noexcept : has_value(o.has_value) {
    if (has_value)
      new (&val) T(std::move(o.val));
    else
      new (&unex) E(std::move(o.unex));
  }

  Result &operator=(Result o) noexcept {
    destroy();
    has_value = o.has_value;
    if (has_value)
      new (&val) T(std::move(o.val));
    else
      new (&unex) E(std::move(o.unex));
    return *this;
  }

  // ── Observers
  [[nodiscard]] bool is_ok() const noexcept { return has_value; }
  [[nodiscard]] bool is_err() const noexcept { return !has_value; }
  explicit operator bool() const noexcept { return has_value; }

  // ── Value access
  auto &unwrap() & {
    assert_ok();
    return val;
  }
  const auto &unwrap() const & {
    assert_ok();
    return val;
  }
  auto unwrap() && {
    assert_ok();
    return std::move(val);
  }

  auto &unwrap_err() & {
    assert_err();
    return unex;
  }
  const auto &unwrap_err() const & {
    assert_err();
    return unex;
  }

  auto value_or(T fallback) const & {
    return has_value ? val : std::move(fallback);
  }
  auto value_or(T fallback) && {
    return has_value ? std::move(val) : std::move(fallback);
  }

  template <typename F>
  auto map(F &&f) const & -> Result<std::invoke_result_t<F, const T &>, E> {
    using U = std::invoke_result_t<F, const T &>;
    if (has_value)
      return Result<U, E>::ok(f(val));
    return Result<U, E>::err(unex);
  }

  template <typename F>
  auto map_err(F &&f) const & -> Result<T, std::invoke_result_t<F, const E &>> {
    using E2 = std::invoke_result_t<F, const E &>;
    if (!has_value)
      return Result<T, E2>::err(f(unex));
    return Result<T, E2>::ok(val);
  }

  template <typename F>
  auto and_then(F &&f) const & -> std::invoke_result_t<F, const T &> {
    using R = std::invoke_result_t<F, const T &>;
    if (has_value)
      return f(val);
    return R::err(unex);
  }

  template <typename F>
  auto or_else(F &&f) const & -> std::invoke_result_t<F, const E &> {
    using R = std::invoke_result_t<F, const E &>;
    if (!has_value)
      return f(unex);
    return R::ok(val);
  }

  template <typename S> auto replace_ok(S &&val) const & -> Result<S, E> {
    using R = Result<S, E>;
    if (!has_value) {
      return R::err(unex);
    }
    return R::ok(std::move(val));
  }

private:
  void assert_ok() const {
    if (!has_value)
      throw std::runtime_error("unwrap() on Err");
  }
  void assert_err() const {
    if (has_value)
      throw std::runtime_error("unwrap_err() on Ok");
  }
};

class IError {
public:
  virtual std::string to_string() const = 0;
  virtual ~IError()                     = default;
};
}  // namespace mjt
template <typename Err>
  requires std::derived_from<Err, mjt::IError>
struct fmt::formatter<Err> : fmt::formatter<std::string_view> {
  auto format(const Err &err, fmt::format_context &ctx) const
    -> fmt::format_context::iterator {
    return fmt::formatter<std::string_view>::format(err.to_string(), ctx);
  }
};
namespace mjt {

//========== Concepts  ==========//

template <typename T>
concept GpuUploadable =
  std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>;

}  // namespace mjt
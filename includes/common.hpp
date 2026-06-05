#pragma once

#include <fmt/core.h>

#include "ansi_char.hpp"

//========== LOG/DEBUG ==========//
#ifndef VERBOSITY
#define VERBOSITY DEBUG
#endif

// -- Log Levels --
enum LogLvl {
  CRITICAL = 50,
  FATAL = CRITICAL,
  ERROR = 40,
  WARNING = 30,
  WARN = WARNING,
  INFO = 20,
  DEBUG = 10,
  NOTSET = 0,
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
      fmt::print("[{}LOG {}] {}::{} ", ansi_code::BBLU, ansi_code::reset,      \
                 __func__, __LINE__);                                          \
      fmt::println(__VA_ARGS__);                                               \
    }                                                                          \
  } while (0)
#endif

#define LOGOK(...)                                                             \
  do {                                                                         \
    fmt::print("[{}OK  {}] {}::{} ", ansi_code::BGRN, ansi_code::reset,        \
               __func__, __LINE__);                                            \
    fmt::println(__VA_ARGS__);                                                 \
  } while (0)

#define LOGWARN(...)                                                           \
  do {                                                                         \
    if (VERBOSITY <= LogLvl::WARNING)                                          \
      fmt::print("[{}WARN{}] {}::{} ", ansi_code::BYEL, ansi_code::reset,     \
                 __func__, __LINE__);                                          \
    fmt::println(__VA_ARGS__);                                                 \
  } while (0)

#define LOGERR(...)                                                            \
  {                                                                            \
    fmt::print("[{}ERR {}] {}::{} ", ansi_code::BRED, ansi_code::reset,        \
               __func__, __LINE__);                                            \
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
ON_DEBUG(x)
#endif

//========== Class Decl Helper  ==========//

#define NO_COPY(CLASS_NAME)                                                    \
  CLASS_NAME(const CLASS_NAME &) = delete;                                     \
  CLASS_NAME &operator=(const CLASS_NAME &) = delete;
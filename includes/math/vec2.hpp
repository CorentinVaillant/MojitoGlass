#pragma once

#include "common.hpp"

namespace mjt {

template <typename T> struct VectorMath2 {
  using This = VectorMath2<T>;

  T x, y;

  constexpr VectorMath2() noexcept = default;
  constexpr VectorMath2(T s) noexcept : x(s), y(s) {}
  constexpr VectorMath2(T x_, T y_) noexcept : x(x_), y(y_) {}

  constexpr auto operator==(This other) noexcept -> bool {
    return x == other.x && y == other.y;
  }
  constexpr auto operator+(This other) noexcept -> This {
    return {x + other.x, y + other.y};
  }
  constexpr auto operator*(This other) noexcept -> This {
    return {x * other.x, y * other.y};
  }
  //...
};

using Vec2    = VectorMath2<float>;
using DVec2   = VectorMath2<double>;

using IVec2   = VectorMath2<int>;
using I8Vec2  = VectorMath2<int8_t>;
using I16Vec2 = VectorMath2<int16_t>;
using I32Vec2 = VectorMath2<int32_t>;
using I64Vec2 = VectorMath2<int64_t>;

using UVec2   = VectorMath2<unsigned int>;
using U8Vec2  = VectorMath2<uint8_t>;
using U16Vec2 = VectorMath2<uint16_t>;
using U32Vec2 = VectorMath2<uint32_t>;
using U64Vec2 = VectorMath2<uint64_t>;
using SVec2   = VectorMath2<size_t>;

}  // namespace mjt
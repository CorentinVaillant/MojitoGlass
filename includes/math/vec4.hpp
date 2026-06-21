#pragma once

#include "common.hpp"

namespace mjt {

template <typename T> struct VectorMath4 {
  using This = VectorMath4<T>;

  T x, y, z, w;

  constexpr VectorMath4() noexcept = default;
  constexpr VectorMath4(T s) noexcept : x(s), y(s) {}
  constexpr VectorMath4(T x_, T y_, T z_, T w_) noexcept
      : x(x_), y(y_), z(z_), w(w_) {}

  constexpr auto operator==(This other) noexcept -> bool {
    return x == other.x && y == other.y && other.z == z && other.w == w;
  }
  constexpr auto operator+(This other) noexcept -> This {
    return {
      x + other.x, y + other.y, z + other.z, w + other.w

    };
  }
  constexpr auto operator*(This other) noexcept -> This {
    return {x * other.x, y * other.y, z * other.z, w * other.w};
  }
  //...
};

using Vec4    = VectorMath4<float>;
using DVec4   = VectorMath4<double>;

using IVec4   = VectorMath4<int>;
using I8Vec4  = VectorMath4<int8_t>;
using I16Vec4 = VectorMath4<int16_t>;
using I32Vec4 = VectorMath4<int32_t>;
using I64Vec4 = VectorMath4<int64_t>;

using UVec4   = VectorMath4<unsigned int>;
using U8Vec4  = VectorMath4<uint8_t>;
using U16Vec4 = VectorMath4<uint16_t>;
using U32Vec4 = VectorMath4<uint32_t>;
using U64Vec4 = VectorMath4<uint64_t>;
using SVec4   = VectorMath4<size_t>;

}  // namespace mjt
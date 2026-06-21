#pragma once

#include <doctest/doctest.h>

#include "math/vec2.hpp"

using namespace mjt;

TEST_CASE("Vec2 tests") {
  Vec2 e = {0, 0};

  SUBCASE("Constr & eq") {
    // Should all be {0,0}
    Vec2 e_1 = {0};
    Vec2 e_2 = {};

    CHECK(e == e_1);
    CHECK(e == e_2);
    CHECK(e_1 == e_2);

    CHECK(Vec2{1} == Vec2{1, 1});
  }

  // TODO more tests !
}
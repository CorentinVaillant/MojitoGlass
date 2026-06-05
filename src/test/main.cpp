#include "math.h"
#include "math/math_test.hpp"

#include "common.hpp"

int main() {
  LOG(INFO, "Starting test :");
  math_test();
  LOGOK("All test good");
  return 0;
}
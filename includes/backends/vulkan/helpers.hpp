#pragma once

#define VK_NO_PROTOTYPES
#include <volk/volk.h>

#include "math/vec4.hpp"
#include <vulkan/vk_enum_string_helper.h>

#define STD140_ALIGNEMENT 16

#define VK_CHECK(x)                                                            \
  do {                                                                         \
    VkResult err = x;                                                          \
                                                                               \
    if (err) {                                                                 \
      LOGERR(                                                                  \
        "Detected Vulkan error: \"{}\" -> {} ", #x, string_VkResult(err));     \
      throw std::runtime_error("VkError");                                     \
    }                                                                          \
  } while (0)

namespace mjt {

static constexpr auto compressed_version_to_uvec4(uint32_t vk_version)
  -> U32Vec4 {
  constexpr uint32_t VARIANT_MASK = 0x7u << 29;
  constexpr uint32_t MAJOR_MASK   = 0x7Fu << 22;
  constexpr uint32_t MINOR_MASK   = 0x3FFu << 12;
  constexpr uint32_t PATCH_MASK   = 0xFFFu;
  static_assert(VARIANT_MASK + MAJOR_MASK + MINOR_MASK + PATCH_MASK == ~0u);

  return {
    vk_version >> 29,
    (vk_version & MAJOR_MASK) >> 22,
    (vk_version & MINOR_MASK) >> 12,
    vk_version & PATCH_MASK,
  };
}

template <
  typename Flag,
  typename FlagBit,
  Flag DefaultVal = static_cast<Flag>(0x0u)>
struct EnumFlagsWrapper {
  Flag flags = DefaultVal;

  constexpr EnumFlagsWrapper(const std::initializer_list<FlagBit> &flags_in) {
    for (auto flag : flags_in) {
      flags |= static_cast<Flag>(flag);
    }
  }

  constexpr EnumFlagsWrapper(FlagBit flag) : flags((static_cast<Flag>(flag))) {}

  constexpr auto operator|=(FlagBit bit) -> EnumFlagsWrapper & {
    flags |= static_cast<Flag>(bit);
    return *this;
  }

  constexpr auto operator|=(EnumFlagsWrapper flags) -> EnumFlagsWrapper & {
    flags |= flags.flags;
    return *this;
  }

  constexpr auto operator|(FlagBit bit) -> EnumFlagsWrapper {
    return {flags | static_cast<Flag>(bit)};
  }

  constexpr auto operator|(EnumFlagsWrapper flags) -> EnumFlagsWrapper {
    return {flags | flags.flags};
  }
};
}  // namespace mjt
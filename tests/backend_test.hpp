#pragma once

#include "backends/vulkan/allocator.hpp"
#include "backends/vulkan_backend.hpp"
#include "common.hpp"
#include "surface/sdl_surface.hpp"

#include <cstdint>
#include <doctest/doctest.h>

using namespace mjt;

TEST_CASE("VkBackend creation + destruction ") {
  SdlSurfaceParams params = SdlSurfaceParams::create_vulkan_presset("test_app");

  auto             surface = SdlSurface::create(params).unwrap();

  VulkanBackendBuilder builder;
  builder.app_name             = "test_app";
  builder.use_validation_layer = true;
  auto backend_result          = VulkanBackend::create(builder, surface);
  CHECK_MESSAGE(
    backend_result.is_ok(),
    "Error while creating vulkan backend got : \n\t",
    backend_result.unwrap_err().to_string());

  auto backend = std::move(backend_result.unwrap());

  SUBCASE("Double VkBackend creation") {
    auto backend_result2 = VulkanBackend::create(builder, surface);
    CHECK_MESSAGE(
      backend_result2.is_ok(),
      "Error while creating vulkan backend got : \n\t",
      backend_result2.unwrap_err().to_string());
    VulkanBackend backend2 = std::move(backend_result2.unwrap());
  }

  SUBCASE("Mem Allocator creation + destruction") {

    AllocatorCreateFlags  flags = {AllocatorCreateBit::ExternallySynchronized};

    VulkanMemoryAllocator allocator = backend.create_memory_allocator(flags);

    SUBCASE("Buffer creation + destruction") {
      struct Foo {
        uint32_t bar1;
        float    bar2;
        char     bar3;
      };

      VulkanBuffer<Foo> buffer = allocator.create_buffer<Foo>(
        10,
        {VulkanBufferUsageBit::StorageBuffer,
         VulkanBufferUsageBit::TransferSrc,
         VulkanBufferUsageBit::TransferDst},
        MemoryUsage::Auto,
        {AllocationCreateBits::Mapped, AllocationCreateBits::HostAccessRandom});

      SUBCASE("Multiple buffers") {
        std::vector<VulkanBuffer<uint8_t>> buffers;
        VulkanBufferUsage                  usage{
          VulkanBufferUsageBit::StorageBuffer,
          VulkanBufferUsageBit::TransferSrc,
          VulkanBufferUsageBit::TransferDst};

        for (int i = 0; i < 10; i++) {
          buffers.emplace_back(
            allocator.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto));
        }
      }

      SUBCASE("Buffer read & write ") {
        std::vector<Foo> foos_src;
        for (int i = 0; i < 10; i++)
          foos_src.push_back({uint32_t(i), float(i), char(i)});
        buffer.write(foos_src);

        buffer.flush();

        std::vector<Foo> foos_dst;
        foos_dst.resize(10);
        buffer.read(foos_dst);

        for (int i = 0; i < 10; i++) {
          CHECK(foos_src[i].bar1 == foos_dst[i].bar1);
          CHECK(foos_src[i].bar2 == foos_dst[i].bar2);
          CHECK(foos_src[i].bar3 == foos_dst[i].bar3);
        }
      }
    }

    SUBCASE("Double mem allocator") {
      VulkanMemoryAllocator allocator2 = backend.create_memory_allocator(flags);

      SUBCASE("Multiple allocator buffers") {
        std::vector<VulkanBuffer<uint8_t>> buffers;
        VulkanBufferUsage usage{VulkanBufferUsageBit::StorageBuffer};
        for (int i = 0; i < 5; i++) {
          buffers.emplace_back(
            allocator.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto));
        }
        for (int i = 0; i < 5; i++) {
          buffers.emplace_back(
            allocator2.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto));
        }
      }
    }
  }
}

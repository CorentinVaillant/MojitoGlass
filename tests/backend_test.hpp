#pragma once

#include "backends/vulkan/allocator.hpp"
#include "backends/vulkan/command_buffer.hpp"
#include "backends/vulkan/command_pool.hpp"
#include "backends/vulkan/fence.hpp"
#include "backends/vulkan/queue.hpp"
#include "backends/vulkan/semaphore.hpp"
#include "backends/vulkan_backend.hpp"
#include "common.hpp"
#include "surface/sdl_surface.hpp"

#include <cstdint>
#include <doctest/doctest.h>

using namespace mjt;

TEST_CASE("VkBackend") {
  SdlSurfaceParams params = SdlSurfaceParams::create_vulkan_presset("test_app");

  auto surface            = SdlSurface::create(params).unwrap();

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

    AllocatorCreateFlags flags = {AllocatorCreateBit::ExternallySynchronized};

    VulkanMemoryAllocator allocator =
      backend.create_memory_allocator(flags).unwrap();

    SUBCASE("Buffer creation + destruction") {
      struct Foo {
        uint32_t bar1;
        float bar2;
        char bar3;
      };

      VulkanBuffer<Foo> buffer = allocator
                                   .create_buffer<Foo>(
                                     10,
                                     {VulkanBufferUsageBit::StorageBuffer,
                                      VulkanBufferUsageBit::TransferSrc,
                                      VulkanBufferUsageBit::TransferDst},
                                     MemoryUsage::Auto,
                                     {AllocationCreateBits::Mapped,
                                      AllocationCreateBits::HostAccessRandom})
                                   .unwrap();

      SUBCASE("Multiple buffers") {
        std::vector<VulkanBuffer<uint8_t>> buffers;
        VulkanBufferUsage usage{
          VulkanBufferUsageBit::StorageBuffer,
          VulkanBufferUsageBit::TransferSrc,
          VulkanBufferUsageBit::TransferDst};

        for (int i = 0; i < 10; i++) {
          buffers.emplace_back(
            allocator.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto)
              .unwrap());
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
      VulkanMemoryAllocator allocator2 =
        backend.create_memory_allocator(flags).unwrap();

      SUBCASE("Multiple allocator buffers") {
        std::vector<VulkanBuffer<uint8_t>> buffers;
        VulkanBufferUsage usage{VulkanBufferUsageBit::StorageBuffer};
        for (int i = 0; i < 5; i++) {
          buffers.emplace_back(
            allocator.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto)
              .unwrap());
        }
        for (int i = 0; i < 5; i++) {
          buffers.emplace_back(
            allocator2.create_buffer<uint8_t>(i + 1, usage, MemoryUsage::Auto)
              .unwrap());
        }
      }
    }
  }

  SUBCASE("Fence creation + destruction") {

    VulkanFence fence = backend.create_fence().unwrap();

    SUBCASE("Fence creation + destruction 2") {
      VulkanFence fence2 = backend.create_fence(true).unwrap();

      SUBCASE("Wait for fences") {
        CHECK(fence.wait(1'000'000).unwrap() == false);
        CHECK(fence2.wait(1'000'000).unwrap() == true);
      }

      SUBCASE("Check signaled") {
        CHECK(fence.signaled().unwrap() == false);
        CHECK(fence2.signaled().unwrap() == true);
      }

      SUBCASE("Fence reset") {
        fence2.reset();
        CHECK(fence2.signaled().unwrap() == false);
      }
    }
  }

  SUBCASE("Semaphore") {
    SUBCASE("Binary semaphore") {
      VulkanBinarySemaphore semaphore =
        backend.create_binary_semaphore().unwrap();

      CHECK(semaphore.raw() != VK_NULL_HANDLE);
    }

    SUBCASE("Timeline Semaphore") {
      VulkanTimelineSemaphore semaphore =
        backend.create_timeline_semaphore(5u).unwrap();

      CHECK(semaphore.query_value().unwrap() == 5);

      constexpr uint64_t WAIT_TIME = 1'000'000;

      CHECK(semaphore.wait(5, WAIT_TIME).unwrap() == true);
      CHECK(semaphore.wait(10, WAIT_TIME).unwrap() == false);
      CHECK(semaphore.wait(100, WAIT_TIME).unwrap() == false);

      semaphore.signal(10);
      CHECK(semaphore.query_value().unwrap() == 10);

      CHECK(semaphore.wait(5, WAIT_TIME).unwrap() == true);
      CHECK(semaphore.wait(10, WAIT_TIME).unwrap() == true);
      CHECK(semaphore.wait(100, WAIT_TIME).unwrap() == false);
    }
  }

  SUBCASE("Queue") {
    auto handle_ret =
      backend.queue_pool().acquire(VulkanQueueFlagBit::Graphics, true);
    CHECK(handle_ret);

    auto &handle = (handle_ret.value());
    CHECK(handle->wait_idle());

    SUBCASE("Command pool") {
      VulkanCmdPool cmd_pool =
        handle->create_cmd_pool({CmdPoolCreateFlagBit::ResetCommandBufferBit})
          .unwrap();

      SUBCASE("Double creation") {
        VulkanCmdPool cmd_pool2 =
          handle->create_cmd_pool({/* CmdPoolCreateFlagBit::TransientBit */})
            .unwrap();
      }

      SUBCASE("Reset") {
        cmd_pool.reset().unwrap();
        cmd_pool.reset(true).unwrap();
      }

      SUBCASE("Command Buffer") {
        VulkanCmd cmd  = cmd_pool.create_cmd().unwrap();
        VulkanCmd cmd2 = cmd_pool.create_cmd(true).unwrap();

        cmd_pool.reset();
        cmd_pool.reset(true);
      }
      SUBCASE("Imediate Cmd") {
        ImediateCmd im_cmd = backend.create_imediate_cmd(cmd_pool).unwrap();

        im_cmd
          .submit([&](auto &cmd) {
            //...
            SUBCASE("Imediate ception") {
              ImediateCmd im_cmd2 =
                backend.create_imediate_cmd(cmd_pool).unwrap();
              im_cmd2.submit([&](auto &cmd) { /*...*/ });
            }
          })
          .unwrap();
      }
    }
  }
}

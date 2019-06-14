#pragma once
#include <vku/vku.hpp>
#include "vk_mem_alloc.h"
#include <cassert>

#include <EASTL\vector.h>

// Generic buffer reconfigured to use VulkanMemoryAllocator
class GenericBufferVMA 
{
public:
  GenericBufferVMA() {}
  GenericBufferVMA(
      VmaAllocator allocator
    , const vk::PhysicalDeviceMemoryProperties & memprops
    , vk::BufferUsageFlags usage
    , vk::DeviceSize size
    , vk::MemoryPropertyFlags memFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
    , VmaAllocationCreateFlags allocFlags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT
    , VmaMemoryUsage vmaMemUsage = VmaMemoryUsage::VMA_MEMORY_USAGE_UNKNOWN
    , VmaPool vmaPool = VK_NULL_HANDLE)
  {
    vk::BufferCreateInfo ci{};
    ci.size = size_ = size;
    ci.usage = usage;
    ci.sharingMode = vk::SharingMode::eExclusive;
    
    VmaAllocationCreateInfo allocInfo = {
      allocFlags,
      vmaMemUsage,
      memFlags.operator VkMemoryMapFlags,
      0,
      0,
      vmaPool,
      nullptr
    };

    VkResult res = vmaCreateBuffer(allocator
      , ci.operator const VkBufferCreateInfo &, &allocInfo
      , reinterpret_cast<VkBuffer*>(&buffer_)
      , &allocation_
      , nullptr
    );
    assert(res == VK_SUCCESS);
  }

  // For host visible buffer
  void updateLocal(VmaAllocator allocator, void * const value, vk::DeviceSize size) 
  {
    void * ptr; 
    vmaMapMemory(allocator, allocation_, &ptr);
    memcpy(ptr, value, static_cast<size_t>(size));
    flush(allocator);
    vmaUnmapMemory(allocator, allocation_);
  }

  template<class Type, class Allocator>
  void updateLocal(VmaAllocator allocator, eastl::vector<Type, Allocator> & value)
  {
    updateLocal(allocator, static_cast<void*>(value.data()), vk::DeviceSize(value.size() * sizeof(Type)));
  }

  template<class Type>
  void updateLocal(VmaAllocator allocator, Type & value)
  {
    updateLocal(allocator, static_cast<void*>(&value), vk::DeviceSize(sizeof(Type)));
  }

  // For device local buffer, depending on queue can stall pipeline
  // Recommend using dedicated transfer queue
  void upload(VmaAllocator allocator, vk::Device device, const vk::PhysicalDeviceMemoryProperties & memprops, vk::CommandPool commandPool, vk::Queue queue, void * value, vk::DeviceSize size)
  {
    if (size == 0) return;
    GenericBufferVMA staging = GenericBufferVMA(
        allocator
      , memprops
      , vk::BufferUsageFlagBits::eTransferSrc
      , size
      , vk::MemoryPropertyFlagBits::eHostVisible
      , VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT
      , VMA_MEMORY_USAGE_CPU_TO_GPU);
    staging.updateLocal(allocator, value, size);

    vku::executeImmediately(device, commandPool, queue, [=, &staging=staging, &buffer_=buffer_](vk::CommandBuffer cb) {
      vk::BufferCopy bc{ 0, 0, size };
      cb.copyBuffer(staging.buffer(), buffer_, bc);
    });
  }

  template<typename T>
  void upload(VmaAllocator allocator, vk::Device device, const vk::PhysicalDeviceMemoryProperties & memprops, vk::CommandPool commandPool, vk::Queue queue, eastl::vector<T> & value)
  {
    upload(allocator, device, memprops, commandPool, queue, value.data(), value.size() * sizeof(T));
  }

  template<typename T>
  void upload(VmaAllocator allocator, vk::Device device, const vk::PhysicalDeviceMemoryProperties & memprops, vk::CommandPool commandPool, vk::Queue queue, T & value)
  {
    upload(allocator, device, memprops, commandPool, queue, &value, sizeof(value));
  }

  void barrier(vk::CommandBuffer cb, vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask, vk::DependencyFlags dependencyFlags, vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
  {
    vk::BufferMemoryBarrier bmb{ srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, buffer_, 0, size_ };
    cb.pipelineBarrier(srcStageMask, dstStageMask, dependencyFlags, nullptr, bmb, nullptr);
  }

  void * map(VmaAllocator allocator)
  {
    void * ptr;
    VkResult result = vmaMapMemory(allocator, allocation_, &ptr);
    assert(result == VK_SUCCESS);
    return ptr;
  }

  void unmap(VmaAllocator allocator)
  {
    vmaUnmapMemory(allocator, allocation_);
  }

  void flush(VmaAllocator allocator) {
    vmaFlushAllocation(allocator, allocation_, 0, VK_WHOLE_SIZE);
  }

  void invalidate(VmaAllocator allocator)
  {
    vmaInvalidateAllocation(allocator, allocation_, 0, VK_WHOLE_SIZE);
  }


  vk::Buffer buffer() const { return buffer_; }
  vk::DeviceSize size() const { return size_; }
  VmaAllocation allocation() const { return allocation_; }
private:
  vk::Buffer buffer_;
  vk::DeviceSize size_;
  VmaAllocation allocation_;
};

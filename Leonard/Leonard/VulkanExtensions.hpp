#pragma once
#include <vulkan/vulkan.h>
#include "UnrecoverableException.hpp"

static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VkResult vkCreateDebugReportCallbackEXT(
  VkInstance                                instance,
  const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks              *pAllocator,
  VkDebugReportCallbackEXT                 *pCallback)
{
  return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
void vkDestroyDebugReportCallbackEXT(
  VkInstance                    instance,
  VkDebugReportCallbackEXT      callback,
  const VkAllocationCallbacks  *pAllocator)
{
  pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

void LoadDebugCallbackFuncs(VkInstance instance)
{
  pfn_vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
  if (pfn_vkCreateDebugReportCallbackEXT == nullptr) 
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Failed to load vkCreateDebugReportCallbackEXT"), "vkGetInstanceProcAddr");
  pfn_vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
  if (pfn_vkCreateDebugReportCallbackEXT == nullptr) 
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Failed to load vkDestroyDebugReportCallbackEXT"), "vkGetInstanceProcAddr");
}


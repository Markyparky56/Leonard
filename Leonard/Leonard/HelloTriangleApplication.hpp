#pragma once
#include <vulkan/vulkan.hpp>
#include <ext_loader\vulkan_ext.h>
#include <GLFW/glfw3.h> // TODO: Replace GLFW with own window manager

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <string>
#include <set>

class HelloTriangleApplication
{
  struct QueueFamilyIndices
  {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete()
    {
      return graphicsFamily >= 0 && presentFamily >= 0;
    }
  };

public:
  void run();

private:
  void initWindow();

  void initVulkan();
  bool checkValidationLayerSupport();
  std::vector<const char*> getRequiredExtensions();
  void listAvailableExtensions();
  void createInstance();
  void setupDebugCallback();
  void removeDebugCallback();
  void pickPhysicalDevice();
  bool isDeviceSuitable(vk::PhysicalDevice device);
  QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
  void createLogicalDevice();
  void createSurface();
  bool checkDeviceExtensionSupport(vk::PhysicalDevice device);

  void mainLoop();

  void cleanup();

  // GLFW stuff
  const int WindowWidth = 800, WindowHeight = 600;
  GLFWwindow *window;

  // Vulkan stuff
  vk::Instance instance;
  vk::DebugReportCallbackEXT callback;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::SurfaceKHR surface;
  vk::Queue presentQueue;

  const std::vector<const char*> validationLayers = 
  {
    "VK_LAYER_LUNARG_standard_validation"
  };

  const std::vector<const char*> deviceExtensions =
  {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };
#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif


  // This callback is the closest to the C-API this program gets, because even the C++-API still falls back on
  // the C-API's typedefs for function pointers.
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugReportFlagsEXT flags
    , VkDebugReportObjectTypeEXT objType
    , uint64_t obj
    , size_t location
    , int32_t code
    , const char *layerPrefix
    , const char *msg
    , void *userData)
  {
    std::cerr << "Validation Layer: " << layerPrefix << " Message: " << msg << std::endl;
    return VK_FALSE;
  }

  // Graphics card stuff
  std::string graphicsCardName;
  uint64_t graphicsCardMemory;
};

#pragma once
#include <vulkan/vulkan.hpp>
#include <ext_loader\vulkan_ext.h>
#include <GLFW/glfw3.h> // TODO: Replace GLFW with own window manager

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>

class HelloTriangleApplication
{
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

  void mainLoop();

  void cleanup();

  // GLFW stuff
  const int WindowWidth = 800, WindowHeight = 600;
  GLFWwindow *window;

  // Vulkan stuff
  vk::Instance instance;
  vk::DebugReportCallbackEXT callback;

  const std::vector<const char*> validationLayers = 
  {
    "VK_LAYER_LUNARG_standard_validation"
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
    std::cerr << "Validation Layer: " << msg << std::endl;
    return VK_FALSE;
  }
};

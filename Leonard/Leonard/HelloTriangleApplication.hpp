#pragma once
//#include <vkel.h>
#include <vulkan/vulkan.hpp>
//#include <ext_loader\vulkan_ext.h>
#include <GLFW/glfw3.h> // TODO: Replace GLFW with own window manager

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "UnrecoverableException.hpp"

#include "Vertex.hpp"
#include "UniformBufferObject.hpp"

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>

static std::vector<char> readBinaryFile(const std::string &filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("Failed to open file!");
  }

  size_t fileSize = static_cast<size_t>(file.tellg());
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

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

  struct SwapChainSupportDetails
  {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
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
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
  void createLogicalDevice();
  void createSurface();
  void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags, vk::MemoryPropertyFlags, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory);
  void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
  bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
  SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);
  vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
  vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes);
  vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
  void createSwapChain();
  void createImageViews();
  void createDescriptorSetLayout();
  void createGraphicsPipeline();
  vk::ShaderModule createShaderModule(const std::vector<char> &code);
  void createRenderPass();
  void createFramebuffers();
  void createCommandPool();
  void createVertexBuffer();
  void createIndexBuffer();
  void createUniformBuffer();
  void createDescriptorPool();
  void createDescriptorSet();
  void createCommandBuffers();
  void createSemaphores();
  void recreateSwapChain();
  void cleanupSwapChain();

  void setupRenderables();

  void mainLoop();
  void updateUniformBuffer();
  void drawFrame();

  void cleanup();

  // GLFW stuff
  const uint32_t WindowWidth = 800, WindowHeight = 600;
  GLFWwindow *window;

  // Vulkan stuff
  vk::Instance instance;
  vk::DebugReportCallbackEXT callback;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  vk::Queue graphicsQueue;
  vk::SurfaceKHR surface;
  vk::Queue presentQueue;
  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::Format swapChainImageFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::ImageView> swapChainImageViews;
  vk::DescriptorSetLayout descriptorSetLayout;
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass renderPass;
  vk::Pipeline graphicsPipeline;
  std::vector<vk::Framebuffer> swapChainFramebuffers;
  vk::DescriptorPool descriptorPool;
  vk::DescriptorSet descriptorSet;
  vk::CommandPool commandPool;
  std::vector<vk::CommandBuffer> commandBuffers;
  vk::Semaphore imageAvailableSemaphore;
  vk::Semaphore renderFinishedSemaphore;

  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  vk::Buffer indexBuffer;
  vk::DeviceMemory indexBufferMemory;
  vk::Buffer uniformBuffer;
  vk::DeviceMemory uniformBufferMemory;

  // Stuff to render
  std::vector<Vertex> vertices;
  std::vector<uint16_t> indices;

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

  static void glfwErrorCallback(
      int error
    , const char *description
  )
  {
    std::cerr << "GLFW Error: " << description << std::endl;
  }

  static void glfwOnWindowResized(
      GLFWwindow *window
    , int width
    , int height
  )
  {
    if (width == 0 || height == 0) return;

    HelloTriangleApplication *app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->recreateSwapChain();
  }

  // Graphics card stuff
  std::string graphicsCardName;
  uint64_t graphicsCardMemory;
};

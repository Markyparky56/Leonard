#include "HelloTriangleApplication.hpp"

void HelloTriangleApplication::run()
{
  initWindow();
  initVulkan();
  mainLoop();
  cleanup();
}

void HelloTriangleApplication::initWindow()
{
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(WindowWidth, WindowHeight, "Leonard", nullptr, nullptr);

}

void HelloTriangleApplication::initVulkan()
{
  createInstance();
  setupDebugCallback();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
  uint32_t layerCount;
  vk::enumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<vk::LayerProperties> availableLayers(layerCount);
  vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  // Check the required validation layers against available layers
  for (const char *layerName : validationLayers)
  {
    bool layerFound = false;

    for (const auto &layerProperties : availableLayers)
    {
      if (strcmp(layerName, layerProperties.layerName) == 0)
      {
        layerFound = true;
        break;
      }
    }

    if (!layerFound)
    {
      return false;
    }
  }

  return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
  std::vector<const char*> extensions;

  unsigned int glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (unsigned int i = 0; i < glfwExtensionCount; i++)
  {
    extensions.push_back(glfwExtensions[i]);
  }

  if (enableValidationLayers)
  {
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  }

  return extensions;
}

void HelloTriangleApplication::listAvailableExtensions()
{
  // Get vulkan extensions
  uint32_t extensionCount = 0;
  vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<vk::ExtensionProperties> extensions(extensionCount);
  vk::enumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

  // Get glfw required extensions
  unsigned int glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // List extensions
  std::cout << "Available Extensions:" << std::endl;
  for (const auto& extension : extensions)
  {
    bool isReqdByGlfw = false;
    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
      if (glfwExtensions[i] == extension.extensionName)
      {
        isReqdByGlfw = true;
        break;
      }
    }
    std::cout << "\t" << extension.extensionName
              << ((isReqdByGlfw) ? " (Required by GLFW)" : "")
              << std::endl;
  }
}

void HelloTriangleApplication::createInstance()
{
  if (enableValidationLayers && !checkValidationLayerSupport())
  {
    throw std::runtime_error("Validation layers requested, but not available!");
  }

  vk::ApplicationInfo appInfo;
  appInfo.setPApplicationName("Hello Triangle")
         .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
         .setPEngineName("No Engine")
         .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
         .setApiVersion(VK_API_VERSION_1_0);

#ifdef _DEBUG
  listAvailableExtensions();
#endif

  vk::InstanceCreateInfo createInfo;
  auto extensions = getRequiredExtensions();
  createInfo.setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
            .setPpEnabledExtensionNames(extensions.data());

  if (enableValidationLayers)
  {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
              .setPpEnabledLayerNames(validationLayers.data());
  }
  else
  {
    createInfo.setEnabledLayerCount(0);
  }

  vk::Result result = vk::createInstance(&createInfo, nullptr, &instance);
  if (result != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create instance!");
  }

  // Use the ext_loader/vulkan_ext loader to fetch the extension functions which aren't loaded by default
  // TODO: Consider moving to after device initialisation as we only have one device
  vkExtInitInstance(instance);
}

void HelloTriangleApplication::setupDebugCallback()
{
  if (!enableValidationLayers) return;

  vk::DebugReportCallbackCreateInfoEXT createInfo;
  createInfo.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning)
            .setPfnCallback(debugCallback);

  if (instance.createDebugReportCallbackEXT(&createInfo, nullptr, &callback) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to setup debug callback!");
  }
}

void HelloTriangleApplication::removeDebugCallback()
{
  if (!enableValidationLayers) return;

  instance.destroyDebugReportCallbackEXT(callback);
}

void HelloTriangleApplication::pickPhysicalDevice()
{
  uint32_t deviceCount;
  instance.enumeratePhysicalDevices(&deviceCount, nullptr);
  
  if (deviceCount == 0)
  {
    throw std::runtime_error("Failed to find GPUs with Vulkan support!");
  }

  std::vector<vk::PhysicalDevice> devices(deviceCount);
  instance.enumeratePhysicalDevices(&deviceCount, devices.data());

  for (const auto& device : devices)
  {
    if (isDeviceSuitable(device))
    {
      physicalDevice = device;
      break;
    }
  }

  if (!physicalDevice)
  {
    throw std::runtime_error("Failed to find a suitable GPU!");
  }
}

bool HelloTriangleApplication::isDeviceSuitable(vk::PhysicalDevice device)
{
  vk::PhysicalDeviceProperties deviceProperties;
  vk::PhysicalDeviceFeatures deviceFeatures;
  vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;
  deviceProperties = device.getProperties();
  deviceFeatures = device.getFeatures();
  deviceMemoryProperties = device.getMemoryProperties();

  // Whatever requirement checks can go here
  QueueFamilyIndices indices = findQueueFamilies(device);

  bool extensionsSupported = checkDeviceExtensionSupport(device);

  bool complete = indices.isComplete() && extensionsSupported;

  if (complete)
  {
    graphicsCardName = deviceProperties.deviceName;
    graphicsCardMemory = static_cast<uint64_t>(deviceMemoryProperties.memoryHeaps[0].size)/1024/1024; // MiB
    std::cout << "Using Physical Device " << graphicsCardName 
              << " with " << graphicsCardMemory << " MiB VRAM" << std::endl;
  }

  return complete;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{
  std::vector<vk::ExtensionProperties> availableExtensions;
  availableExtensions = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extensions : availableExtensions)
  {
    requiredExtensions.erase(extensions.extensionName);
  }

  return requiredExtensions.empty();
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(vk::PhysicalDevice device)
{
  QueueFamilyIndices indices;  

  std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

  int i = 0;
  for (const auto &queueFamily : queueFamilies)
  {
    if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphicsFamily = i;
    }

    vk::Bool32 presentSupport = false;
    device.getSurfaceSupportKHR(i, surface, &presentSupport);

    if (queueFamily.queueCount > 0 && presentSupport)
    {
      indices.presentFamily = i;
    }

    if (indices.isComplete())
    {
      break;
    }
    i++;
  }

  return indices;
}

void HelloTriangleApplication::createLogicalDevice()
{
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  //float queuePriority = 1.f;

  //vk::DeviceQueueCreateInfo queueCreateInfo;
  //queueCreateInfo.setQueueFamilyIndex(indices.graphicsFamily)
  //               .setQueueCount(1)
  //               .setPQueuePriorities(&queuePriority);

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
  std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

  float queuePriority = 1.f;
  for (int queueFamily : uniqueQueueFamilies)
  {
    vk::DeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.setQueueFamilyIndex(queueFamily)
                   .setQueueCount(1)
                   .setPQueuePriorities(&queuePriority);
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::PhysicalDeviceFeatures deviceFeatures;
  
  vk::DeviceCreateInfo createInfo;
  createInfo.setPQueueCreateInfos(queueCreateInfos.data())
            .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
            .setPpEnabledExtensionNames(deviceExtensions.data());

  if (enableValidationLayers)
  {
    createInfo.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
              .setPpEnabledLayerNames(validationLayers.data());
  }
  else
  {
    createInfo.setEnabledLayerCount(0);
  }

  if (physicalDevice.createDevice(&createInfo, nullptr, &device) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create logical device!");
  }

  device.getQueue(indices.graphicsFamily, 0, &graphicsQueue);
  device.getQueue(indices.presentFamily, 0, &presentQueue);
}

void HelloTriangleApplication::createSurface()
{
  if (glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface!");
  }
}

void HelloTriangleApplication::mainLoop()
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();
  }
}

void HelloTriangleApplication::cleanup()
{
  device.destroy();
  removeDebugCallback();
  instance.destroySurfaceKHR(surface);
  instance.destroy();

  glfwDestroyWindow(window);
  glfwTerminate();
}

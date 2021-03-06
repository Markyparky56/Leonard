#include "HelloTriangleApplication.hpp"
#include "VulkanExtensions.hpp"

// TODO: Check which exception throws are actually necessary given Vulkan-Hpp also checks for exceptions

void HelloTriangleApplication::run()
{
  initWindow();
  setupRenderables(); // Simple function to initialise vertices array
  initVulkan();
  mainLoop();
  cleanup();
}

void HelloTriangleApplication::setupRenderables()
{
  // Rainbow Triangle Vertices
  /*vertices = {
    {{ 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}}
  };*/
  // Square Vertices
  vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
  };

  // Square Indices
  indices = {
    0, 1, 2, 2, 3, 0
  };
}

void HelloTriangleApplication::initWindow()
{
  glfwSetErrorCallback(glfwErrorCallback);
  if (glfwInit() == GLFW_TRUE)
  {
    auto vulkanSupported = glfwVulkanSupported();
    std::cout << "Vulkan Supported: " << ((vulkanSupported) ? "True" : "False") << std::endl;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WindowWidth, WindowHeight, "Leonard", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, HelloTriangleApplication::glfwOnWindowResized);
  }
  else
  {
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("GLFW Failed to initialise!"), "glfwInit");
  }
}

void HelloTriangleApplication::initVulkan()
{
  createInstance();
  setupDebugCallback();
  createSurface();
  pickPhysicalDevice();
  createLogicalDevice();
  createSwapChain();
  createImageViews();
  createRenderPass();
  createDescriptorSetLayout();
  createGraphicsPipeline();
  createFramebuffers();
  createCommandPool();
  createVertexBuffer();
  createIndexBuffer();
  createUniformBuffer();
  createDescriptorPool();
  createDescriptorSet();
  createCommandBuffers();
  createSemaphores();
}

bool HelloTriangleApplication::checkValidationLayerSupport()
{
  std::vector<vk::LayerProperties> availableLayers;
  availableLayers = vk::enumerateInstanceLayerProperties();

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

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  for (uint32_t i = 0; i < glfwExtensionCount; i++)
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
  std::vector<vk::ExtensionProperties> extensions = vk::enumerateInstanceExtensionProperties();

  // Get glfw required extensions
  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  // List extensions
  std::cout << "Available Extensions:" << std::endl;
  for (const auto& extension : extensions)
  {
    bool isReqdByGlfw = false;
    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
      if(strcmp(glfwExtensions[i], extension.extensionName) == 0)
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
    cleanup();
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Validation layers requested but not available!"), "checkValidationLayerSupport");
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

  try
  {
    instance = vk::createInstance(createInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create instance!"), e);
  }

  LoadDebugCallbackFuncs(instance);
}

void HelloTriangleApplication::setupDebugCallback()
{
  if (!enableValidationLayers) return;

  vk::DebugReportCallbackCreateInfoEXT createInfo;
  createInfo.setFlags(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning)
            .setPfnCallback(debugCallback);

  try
  {
    callback = instance.createDebugReportCallbackEXT(createInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to setup debug callback!"), e);
  }
}

void HelloTriangleApplication::removeDebugCallback()
{
  if (!enableValidationLayers) return;

  instance.destroyDebugReportCallbackEXT(callback);
}

void HelloTriangleApplication::pickPhysicalDevice()
{
  std::vector<vk::PhysicalDevice> devices;
  try
  {
    devices = instance.enumeratePhysicalDevices();
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to enumerate physical devices!"), e);
  }

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
    cleanup();
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Failed to find a suitable GPU!"), "pickPhysicalDevice");
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

  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
  }

  bool complete = indices.isComplete() && extensionsSupported && swapChainAdequate;

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
  try
  {
    availableExtensions = device.enumerateDeviceExtensionProperties();
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to enumerate device extension properties!"), e);
  }

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto &extensions : availableExtensions)
  {
    requiredExtensions.erase(extensions.extensionName);
  }

  return requiredExtensions.empty();
}

HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(vk::PhysicalDevice device)
{
  SwapChainSupportDetails details;

  try { details.capabilities = device.getSurfaceCapabilitiesKHR(surface); }
  catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to get surface capabilities!"), e); }
  try { details.formats = device.getSurfaceFormatsKHR(surface); }
  catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to get surface formats!"), e); }
  try { details.presentModes = device.getSurfacePresentModesKHR(surface); }
  catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to get surface present modes!"), e); }

  return details;
}

vk::SurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
  if (availableFormats.size() == 1 && availableFormats.begin()->format == vk::Format::eUndefined)
  {
    return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
  }

  for (const auto& availableFormat : availableFormats)
  {
    if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
    {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

vk::PresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
  vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

  for (const auto& availablePresentMode : availablePresentModes)
  {
    if (availablePresentMode == vk::PresentModeKHR::eMailbox)
    {
      return availablePresentMode;
    }
    else if (availablePresentMode == vk::PresentModeKHR::eImmediate)
    {
      bestMode = availablePresentMode;
    }
  }

  return bestMode;
}

vk::Extent2D HelloTriangleApplication::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR & capabilities)
{
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
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

    vk::Bool32 presentSupport;
    try { presentSupport = device.getSurfaceSupportKHR(i, surface); }
    catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to get surface support!"), e); }

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

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
  vk::PhysicalDeviceMemoryProperties memProperties;
  memProperties = physicalDevice.getMemoryProperties();

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
  {
    if ((typeFilter & (1 << i))
    && ((memProperties.memoryTypes[i].propertyFlags & properties) == properties))
    {
      return i;
    }
  }

  throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Failed to find suitable memory type!"), "findMemoryType");
}

void HelloTriangleApplication::createLogicalDevice()
{
  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

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

  try
  {
    device = physicalDevice.createDevice(createInfo);
  }
  catch(std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create logical device!"), e);
  }

  graphicsQueue = device.getQueue(indices.graphicsFamily, 0);
  presentQueue = device.getQueue(indices.presentFamily, 0);
}

void HelloTriangleApplication::createSurface()
{
  VkResult result = glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));
  if (result != VK_SUCCESS)
  {
    cleanup();
    throw UnrecoverableRuntimeException(CreateBasicExceptionMessage("Failed to create window surface!"), "glfwCreateWindowSurface");
  }
}

void HelloTriangleApplication::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer & buffer, vk::DeviceMemory & bufferMemory)
{
  vk::BufferCreateInfo bufferInfo = {};
  bufferInfo.setSize(size)
            .setUsage(usage)
            .setSharingMode(vk::SharingMode::eExclusive);

  try
  {
    buffer = device.createBuffer(bufferInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create vertex buffer!"), e);
  }

  vk::MemoryRequirements memRequirements;
  memRequirements = device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocInfo = {};
  allocInfo.setAllocationSize(memRequirements.size)
           .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, properties));

  try
  {
    bufferMemory = device.allocateMemory(allocInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to allocate vertex buffer memory!"), e);
  }

  device.bindBufferMemory(buffer, bufferMemory, 0);
}

void HelloTriangleApplication::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
  vk::CommandBufferAllocateInfo allocInfo = {};
  allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
           .setCommandPool(commandPool)
           .setCommandBufferCount(1);

  vk::CommandBuffer commandBuffer;
  device.allocateCommandBuffers(&allocInfo, &commandBuffer);

  vk::CommandBufferBeginInfo beginInfo = {};
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  commandBuffer.begin(beginInfo);
  
  vk::BufferCopy copyRegion = {};
  copyRegion.setSrcOffset(0)
            .setDstOffset(0)
            .setSize(size);
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

  commandBuffer.end();

  vk::SubmitInfo submitInfo = {};
  submitInfo.setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffer);

  try 
  { 
    graphicsQueue.submit(1, &submitInfo, nullptr); 
    graphicsQueue.waitIdle(); 
  }
  catch (std::system_error const &e) 
  { 
    cleanup(); 
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to submit copy buffer command to graphics queue!"), e); 
  }

  device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::createSwapChain()
{
  SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

  vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
  vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
  vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  vk::SwapchainCreateInfoKHR createInfo;
  createInfo.setSurface(surface)
            .setMinImageCount(imageCount)
            .setImageFormat(surfaceFormat.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(extent)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);  
            // vk::ImageUsageFlagBits::eTransferDst for post-processing

  QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
  uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };  

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
              .setQueueFamilyIndexCount(2)
              .setPQueueFamilyIndices(queueFamilyIndices);
  }
  else
  {
    createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
              .setQueueFamilyIndexCount(0)
              .setPQueueFamilyIndices(nullptr);
  }

  createInfo.setPreTransform(swapChainSupport.capabilities.currentTransform)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setPresentMode(presentMode)
            .setClipped(true)
            .setOldSwapchain(nullptr);

  try
  {
    swapChain = device.createSwapchainKHR(createInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create swap chain!"), e);
  }

  try { swapChainImages = device.getSwapchainImagesKHR(swapChain); }
  catch (std::system_error const &e) 
  { 
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to get swapchain images!"), e);
  }
  swapChainImageFormat = surfaceFormat.format;
  swapChainExtent = extent;
}

void HelloTriangleApplication::createImageViews()
{
  swapChainImageViews.resize(swapChainImages.size());
  for (size_t i = 0; i < swapChainImages.size(); i++)
  {
    vk::ImageViewCreateInfo createInfo;

    createInfo.setImage(swapChainImages[i])
              .setViewType(vk::ImageViewType::e2D)
              .setFormat(swapChainImageFormat)
              .setComponents({ 
                              vk::ComponentSwizzle::eIdentity, // r
                              vk::ComponentSwizzle::eIdentity, // g
                              vk::ComponentSwizzle::eIdentity, // b
                              vk::ComponentSwizzle::eIdentity  // a
                            })
              .setSubresourceRange({
                                    vk::ImageAspectFlagBits::eColor, // Aspect Mask
                                    0U, // Base Mip Level
                                    1U, // Level Count
                                    0U, // Base Array Layer
                                    1U  // Layer Count
                                  });

    try
    {
      swapChainImageViews[i] = device.createImageView(createInfo);
    }
    catch (std::system_error const &e)
    {
      cleanup();
      throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create image views!"), e);
    }
  }
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
  vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
  uboLayoutBinding.setBinding(0)
                  .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                  .setDescriptorCount(1)
                  .setStageFlags(vk::ShaderStageFlagBits::eVertex)
                  .setPImmutableSamplers(nullptr);

  vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
  layoutInfo.setBindingCount(1)
    .setPBindings(&uboLayoutBinding);

  try
  {
    descriptorSetLayout = device.createDescriptorSetLayout(layoutInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create descriptor set layout!"), e);
  }
}

void HelloTriangleApplication::createGraphicsPipeline()
{
  vk::ShaderModule vertShaderModule;
  vk::ShaderModule fragShaderModule;
  auto vertShaderCode = readBinaryFile("shaders/triangle.vert.spv");
  auto fragShaderCode = readBinaryFile("shaders/triangle.frag.spv");
#if defined(_DEBUG)
  std::cout << "Vert shader code size: " << vertShaderCode.size() << std::endl;
  std::cout << "Frag shader code size: " << fragShaderCode.size() << std::endl;
#endif // defined(_DEBUG)

  vertShaderModule = createShaderModule(vertShaderCode);
  fragShaderModule = createShaderModule(fragShaderCode);

  vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
  vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex)
                     .setModule(vertShaderModule)
                     .setPName("main");

  vk::PipelineShaderStageCreateInfo fragshaderStageInfo;
  fragshaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment)
                     .setModule(fragShaderModule)
                     .setPName("main");

  vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragshaderStageInfo };

  auto bindingDescription = Vertex::getBindingDescription();
  auto attributeDescriptions = Vertex::getAttributeDescriptions();

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.setVertexBindingDescriptionCount(1)
                 .setPVertexBindingDescriptions(&bindingDescription)
                 .setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
                 .setPVertexAttributeDescriptions(attributeDescriptions.data());

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
  inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList)
               .setPrimitiveRestartEnable(false);

  vk::Viewport viewport;
  viewport.setX(0.f)
          .setY(0.f)
          .setWidth(static_cast<float>(swapChainExtent.width))
          .setHeight(static_cast<float>(swapChainExtent.height))
          .setMinDepth(0.f)
          .setMaxDepth(1.f);

  vk::Rect2D scissor;
  scissor.setOffset({ 0,0 })
         .setExtent(swapChainExtent);

  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.setViewportCount(1)
               .setPViewports(&viewport)
               .setScissorCount(1)
               .setPScissors(&scissor);

  vk::PipelineRasterizationStateCreateInfo rasterizer;
  rasterizer.setDepthClampEnable(false)
            .setRasterizerDiscardEnable(false)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.f)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(false)
            .setDepthBiasConstantFactor(0.f)
            .setDepthBiasClamp(0.f)
            .setDepthBiasSlopeFactor(0.f);

  vk::PipelineMultisampleStateCreateInfo multisampling;
  multisampling.setSampleShadingEnable(false)
               .setRasterizationSamples(vk::SampleCountFlagBits::e1)
               .setMinSampleShading(1.f)
               .setPSampleMask(nullptr)
               .setAlphaToCoverageEnable(false)
               .setAlphaToOneEnable(false);

  vk::PipelineColorBlendAttachmentState colorBlendAttachment;
  colorBlendAttachment.setColorWriteMask( vk::ColorComponentFlagBits::eR 
                                        | vk::ColorComponentFlagBits::eG 
                                        | vk::ColorComponentFlagBits::eB 
                                        | vk::ColorComponentFlagBits::eA)
                      //.setBlendEnable(false)
                      //.setSrcColorBlendFactor(vk::BlendFactor::eOne)
                      //.setDstColorBlendFactor(vk::BlendFactor::eZero)
                      //.setColorBlendOp(vk::BlendOp::eAdd)
                      //.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                      //.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                      //.setAlphaBlendOp(vk::BlendOp::eAdd);
                        .setBlendEnable(true)
                        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                        .setColorBlendOp(vk::BlendOp::eAdd)
                        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                        .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
                        .setAlphaBlendOp(vk::BlendOp::eAdd);

  vk::PipelineColorBlendStateCreateInfo colorBlending;
  colorBlending.setLogicOpEnable(false)
               .setLogicOp(vk::LogicOp::eCopy)
               .setAttachmentCount(1)
               .setPAttachments(&colorBlendAttachment)
               .setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

  std::vector<vk::DynamicState> dynamicStates =
  {
    vk::DynamicState::eViewport,
    vk::DynamicState::eLineWidth
  };

  vk::PipelineDynamicStateCreateInfo dynamicState;
  dynamicState.setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
    .setPDynamicStates(dynamicStates.data());

  
  vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
  pipelineLayoutInfo.setSetLayoutCount(1)
                    .setPSetLayouts(&descriptorSetLayout)
                    .setPushConstantRangeCount(0)
                    .setPPushConstantRanges(0);
  
  try
  {
    pipelineLayout = device.createPipelineLayout(pipelineLayoutInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create pipeline layout!"), e);
  }

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.setStageCount(2)
              .setPStages(shaderStages)
              .setPVertexInputState(&vertexInputInfo)
              .setPInputAssemblyState(&inputAssembly)
              .setPViewportState(&viewportState)
              .setPRasterizationState(&rasterizer)
              .setPMultisampleState(&multisampling)
              .setPDepthStencilState(nullptr)
              .setPColorBlendState(&colorBlending)
              .setPDynamicState(nullptr)
              .setLayout(pipelineLayout)
              .setRenderPass(renderPass)
              .setSubpass(0)
              .setBasePipelineHandle(nullptr)
              .setBasePipelineIndex(-1);

  try
  {
    graphicsPipeline = device.createGraphicsPipeline(nullptr, pipelineInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create graphics pipeline!"), e);
  }
  
  device.destroyShaderModule(vertShaderModule);
  device.destroyShaderModule(fragShaderModule);
}

void HelloTriangleApplication::createRenderPass()
{
  vk::AttachmentDescription colorAttachment;
  colorAttachment.setFormat(swapChainImageFormat)
                 .setSamples(vk::SampleCountFlagBits::e1)
                 .setLoadOp(vk::AttachmentLoadOp::eClear)
                 .setStoreOp(vk::AttachmentStoreOp::eStore)
                 .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                 .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
                 .setInitialLayout(vk::ImageLayout::eUndefined)
                 .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference colorAttachmentRef;
  colorAttachmentRef.setAttachment(0) // corresponds to layout(location = 0) in fragment shader
                    .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass;
  subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
         .setColorAttachmentCount(1)
         .setPColorAttachments(&colorAttachmentRef);

  vk::SubpassDependency dependency;
  dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setSrcAccessMask(vk::AccessFlags())
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead
                            | vk::AccessFlagBits::eColorAttachmentWrite);

  vk::RenderPassCreateInfo renderPassInfo;
  renderPassInfo.setAttachmentCount(1)
                .setPAttachments(&colorAttachment)
                .setSubpassCount(1)
                .setPSubpasses(&subpass)
                .setDependencyCount(1)
                .setPDependencies(&dependency);

  try
  {
    renderPass = device.createRenderPass(renderPassInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create render pass!"), e);
  }
}

vk::ShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCodeSize(code.size())
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
  vk::ShaderModule shaderModule;
  try
  {
    shaderModule = device.createShaderModule(createInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create shader module!"), e);
  }
  return shaderModule;
}

void HelloTriangleApplication::createFramebuffers()
{
  swapChainFramebuffers.resize(swapChainImageViews.size());

  for (size_t i = 0; i < swapChainImageViews.size(); i++)
  {
    vk::ImageView attachments[] =
    {
      swapChainImageViews[i]
    };

    vk::FramebufferCreateInfo framebufferInfo;
    framebufferInfo.setRenderPass(renderPass)
                   .setAttachmentCount(1)
                   .setPAttachments(attachments)
                   .setWidth(swapChainExtent.width)
                   .setHeight(swapChainExtent.height)
                   .setLayers(1);

    try
    {
      swapChainFramebuffers[i] = device.createFramebuffer(framebufferInfo);
    }
    catch (std::system_error const &e)
    {
      cleanup();
      throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create framebuffer!"), e);
    }
  }
}

void HelloTriangleApplication::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  vk::CommandPoolCreateInfo poolInfo;
  poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily);

  try
  {
    commandPool = device.createCommandPool(poolInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create command pool!"), e);
  }
}

void HelloTriangleApplication::createVertexBuffer()
{
  vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

  vk::Buffer stagingBuffer;
  vk::DeviceMemory stagingBufferMemory;
  createBuffer( bufferSize
              , vk::BufferUsageFlagBits::eTransferSrc
              , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
              , stagingBuffer, stagingBufferMemory);

  void *data;
  data = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  device.unmapMemory(stagingBufferMemory);

  createBuffer( bufferSize
              , vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
              , vk::MemoryPropertyFlagBits::eDeviceLocal
              , vertexBuffer, vertexBufferMemory); 

  copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);
}

void HelloTriangleApplication::createIndexBuffer()
{
  vk::DeviceSize bufferSize = sizeof(uint16_t) * indices.size();

  vk::Buffer stagingBuffer;
  vk::DeviceMemory stagingBufferMemory;
  createBuffer( bufferSize
              , vk::BufferUsageFlagBits::eTransferSrc
              , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
              , stagingBuffer, stagingBufferMemory);

  void *data;
  data = device.mapMemory(stagingBufferMemory, 0, bufferSize);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  device.unmapMemory(stagingBufferMemory);

  createBuffer( bufferSize
              , vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
              , vk::MemoryPropertyFlagBits::eDeviceLocal
              , indexBuffer, indexBufferMemory);

  copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  device.destroyBuffer(stagingBuffer);
  device.freeMemory(stagingBufferMemory);
}

void HelloTriangleApplication::createUniformBuffer()
{
  vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
  createBuffer( bufferSize
              , vk::BufferUsageFlagBits::eUniformBuffer
              , vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
              , uniformBuffer, uniformBufferMemory);
}

void HelloTriangleApplication::createDescriptorPool()
{
  vk::DescriptorPoolSize poolSize = {};
  poolSize.setDescriptorCount(1)
          .setType(vk::DescriptorType::eUniformBuffer);

  vk::DescriptorPoolCreateInfo poolInfo = {};
  poolInfo.setPoolSizeCount(1)
          .setPPoolSizes(&poolSize)
          .setMaxSets(1);

  try
  {
    descriptorPool = device.createDescriptorPool(poolInfo);
  }
  catch (std::system_error const &e)
  {
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create descriptor pool!"), e);
  }
}

void HelloTriangleApplication::createDescriptorSet()
{
  vk::DescriptorSetLayout layouts[] = { descriptorSetLayout };
  vk::DescriptorSetAllocateInfo allocInfo = {};
  allocInfo.setDescriptorPool(descriptorPool)
    .setDescriptorSetCount(1)
    .setPSetLayouts(layouts);

  try
  {
    descriptorSet = device.allocateDescriptorSets(allocInfo)[0];
  }
  catch (std::system_error const &e)
  {
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to allocate descriptor set!"), e);
  }

  vk::DescriptorBufferInfo bufferInfo = {};
  bufferInfo.setBuffer(uniformBuffer)
    .setOffset(0)
    .setRange(sizeof(UniformBufferObject));

  vk::WriteDescriptorSet descriptorWrite = {};
  descriptorWrite.setDstSet(descriptorSet)
                 .setDstBinding(0)
                 .setDstArrayElement(0)
                 .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                 .setDescriptorCount(1)
                 .setPBufferInfo(&bufferInfo)
                 .setPImageInfo(nullptr)
                 .setPTexelBufferView(nullptr);

  device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void HelloTriangleApplication::createCommandBuffers()
{
  commandBuffers.resize(swapChainFramebuffers.size());

  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setCommandPool(commandPool)
           .setLevel(vk::CommandBufferLevel::ePrimary)
           .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

  try
  {
    commandBuffers = device.allocateCommandBuffers(allocInfo);
  }
  catch (std::system_error const &e)
  {
    cleanup();
    throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to allocate command buffers!"), e);
  }

  for (size_t i = 0; i < commandBuffers.size(); i++)
  {
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse)
             .setPInheritanceInfo(nullptr);

    commandBuffers[i].begin(&beginInfo);

    vk::ClearValue clearColor(std::array<float, 4Ui64>({ 0.f, 0.f, 0.f, 1.f }));

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.setRenderPass(renderPass)
                  .setFramebuffer(swapChainFramebuffers[i])
                  .setRenderArea(vk::Rect2D({ 0,0 }, swapChainExtent))
                  .setClearValueCount(1)
                  .setPClearValues(&clearColor);

    commandBuffers[i].beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

    commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Buffer vertexBuffers[] = { vertexBuffer };
    vk::DeviceSize offsets[] = { 0 };
    commandBuffers[i].bindVertexBuffers(0, 1, vertexBuffers, offsets);
    commandBuffers[i].bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);
    commandBuffers[i].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    commandBuffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    commandBuffers[i].endRenderPass();
    commandBuffers[i].end();
  }  
}

void HelloTriangleApplication::createSemaphores()
{
  vk::SemaphoreCreateInfo semaphoreInfo;

  try { imageAvailableSemaphore = device.createSemaphore(semaphoreInfo); }
  catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create imageAvailableSemaphore!"), e); }
  try { renderFinishedSemaphore = device.createSemaphore(semaphoreInfo); }
  catch (std::system_error const &e) { cleanup();  throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to create renderFinishedSemaphore!"), e); }
}

void HelloTriangleApplication::recreateSwapChain()
{
  device.waitIdle();

  cleanupSwapChain();

  createSwapChain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
  createFramebuffers();
  createCommandBuffers();
}

void HelloTriangleApplication::cleanupSwapChain()
{
  for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
  {
    if (swapChainFramebuffers[i]) device.destroyFramebuffer(swapChainFramebuffers[i]);
  }
  device.freeCommandBuffers(commandPool, commandBuffers);
  if (graphicsPipeline) device.destroyPipeline(graphicsPipeline);
  if (pipelineLayout) device.destroyPipelineLayout(pipelineLayout);
  if (renderPass) device.destroyRenderPass(renderPass);
  for (size_t i = 0; i < swapChainImageViews.size(); i++)
  {
    if (swapChainImageViews[i]) device.destroyImageView(swapChainImageViews[i]);
  }
  if (swapChain) device.destroySwapchainKHR(swapChain);
}

void HelloTriangleApplication::mainLoop()
{
  while (!glfwWindowShouldClose(window))
  {
    glfwPollEvents();

    updateUniformBuffer();
    drawFrame();
  }

  device.waitIdle();
}

void HelloTriangleApplication::updateUniformBuffer()
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

  UniformBufferObject ubo = {};
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
  // UnInvert Y coords
  ubo.proj[1][1] *= -1;

  void *data;
  data = device.mapMemory(uniformBufferMemory, 0, sizeof(ubo));
  memcpy(data, &ubo, sizeof(ubo));
  device.unmapMemory(uniformBufferMemory);
}

void HelloTriangleApplication::drawFrame()
{
  uint32_t imageIndex;
  try
  {
    auto imageIndexResult = device.acquireNextImageKHR(swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, nullptr);
    imageIndex = imageIndexResult.value;
  }
  catch (std::system_error const &e)
  {
    if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR))
    {
      recreateSwapChain();
      return;
    }
    else
    {
      cleanup();
      throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to acquire swap chain image!"), e);
    }
  }  

  vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
  vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };

  vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
  vk::SubmitInfo submitInfo;
  submitInfo.setWaitSemaphoreCount(1)
            .setPWaitSemaphores(waitSemaphores)
            .setPWaitDstStageMask(waitStages)
            .setCommandBufferCount(1)
            .setPCommandBuffers(&commandBuffers[imageIndex])
            .setSignalSemaphoreCount(1)
            .setPSignalSemaphores(signalSemaphores);

  try { graphicsQueue.submit(submitInfo, nullptr); }
  catch (std::system_error const &e) { cleanup(); throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to submit to graphics queue!"), e); }

  vk::SwapchainKHR swapChains[] = { swapChain };
  vk::PresentInfoKHR presentInfo;
  presentInfo.setWaitSemaphoreCount(1)
             .setPWaitSemaphores(signalSemaphores)
             .setSwapchainCount(1)
             .setPSwapchains(swapChains)
             .setPImageIndices(&imageIndex)
             .setPResults(nullptr);

  try
  {
    presentQueue.presentKHR(presentInfo);
  }
  catch (std::system_error const &e)
  {
    if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR))
    {
      recreateSwapChain();
    }
    else
    {
      cleanup();
      throw UnrecoverableVulkanException(CreateBasicExceptionMessage("Failed to present swap chain image!"), e);
    }
  }
  presentQueue.waitIdle();
}

void HelloTriangleApplication::cleanup()
{
  cleanupSwapChain();

  if (imageAvailableSemaphore)  device.destroySemaphore(imageAvailableSemaphore);
  if (renderFinishedSemaphore)  device.destroySemaphore(renderFinishedSemaphore);
  if (commandPool)              device.destroyCommandPool(commandPool);  
  if (vertexBuffer)             device.destroyBuffer(vertexBuffer);
  if (vertexBufferMemory)       device.freeMemory(vertexBufferMemory);
  if (indexBuffer)              device.destroyBuffer(indexBuffer);
  if (indexBufferMemory)        device.freeMemory(indexBufferMemory);
  if (uniformBuffer)            device.destroyBuffer(uniformBuffer);
  if (uniformBufferMemory)      device.freeMemory(uniformBufferMemory);
  if (descriptorSetLayout)      device.destroyDescriptorSetLayout(descriptorSetLayout);
  if (descriptorPool)           device.destroyDescriptorPool(descriptorPool);
  if (device)                   device.destroy();
  if (callback)                 removeDebugCallback();
  if (surface)                  instance.destroySurfaceKHR(surface);
  if (instance)                 instance.destroy();

  glfwDestroyWindow(window);
  glfwTerminate();
}

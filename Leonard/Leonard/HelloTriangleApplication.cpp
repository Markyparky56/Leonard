#include "HelloTriangleApplication.hpp"

// TODO: Check which exception throws are actually necessary given Vulkan-Hpp also checks for exceptions

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
  createSwapChain();
  createImageViews();
  createRenderPass();
  createGraphicsPipeline();
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
  availableExtensions = device.enumerateDeviceExtensionProperties();

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

  details.capabilities = device.getSurfaceCapabilitiesKHR(surface);
  details.formats = device.getSurfaceFormatsKHR(surface);
  details.presentModes = device.getSurfacePresentModesKHR(surface);

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
    vk::Extent2D actualExtent = { WindowWidth, WindowHeight };

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
  //vkExtInitDevice(device);
}

void HelloTriangleApplication::createSurface()
{
  if (glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface!");
  }
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

  if (device.createSwapchainKHR(&createInfo, nullptr, &swapChain) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create swap chain!");
  }

  swapChainImages = device.getSwapchainImagesKHR(swapChain);
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

    if (device.createImageView(&createInfo, nullptr, &swapChainImageViews[i]) != vk::Result::eSuccess)
    {
      throw std::runtime_error("Failed to create image views!");
    }
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

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.setVertexBindingDescriptionCount(0)
                 .setPVertexBindingDescriptions(nullptr)
                 .setVertexAttributeDescriptionCount(0)
                 .setPVertexAttributeDescriptions(nullptr);

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
            .setFrontFace(vk::FrontFace::eClockwise)
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
  pipelineLayoutInfo.setSetLayoutCount(0)
                    .setPSetLayouts(nullptr)
                    .setPushConstantRangeCount(0)
                    .setPPushConstantRanges(0);
    
  if (device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create pipeline layout!");
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

  if (device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create graphics pipeline!");
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

  vk::RenderPassCreateInfo renderPassInfo;
  renderPassInfo.setAttachmentCount(1)
    .setPAttachments(&colorAttachment)
    .setSubpassCount(1)
    .setPSubpasses(&subpass);

  if (device.createRenderPass(&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create render pass!");
  }
}

vk::ShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code)
{
  vk::ShaderModuleCreateInfo createInfo;
  createInfo.setCodeSize(code.size())
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()));
  vk::ShaderModule shaderModule;
  if (device.createShaderModule(&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create shader module!");
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

    if (device.createFramebuffer(&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess)
    {
      throw std::runtime_error("Failed to create framebuffer!");
    }
  }
}

void HelloTriangleApplication::createCommandPool()
{
  QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

  vk::CommandPoolCreateInfo poolInfo;
  poolInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsFamily);

  if (device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create command pool!");
  }
}

void HelloTriangleApplication::createCommandBuffers()
{
  commandBuffers.resize(swapChainFramebuffers.size());

  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setCommandPool(commandPool)
    .setLevel(vk::CommandBufferLevel::ePrimary)
    .setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));

  if (device.allocateCommandBuffers(&allocInfo, commandBuffers.data()) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to allocate command buffers!");
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
    commandBuffers[i].draw(3, 1, 0, 0);

    commandBuffers[i].endRenderPass();
    commandBuffers[i].end();
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
  for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
  {
    device.destroyFramebuffer(swapChainFramebuffers[i]);
  }
  for (size_t i = 0; i < swapChainImageViews.size(); i++)
  {
    device.destroyImageView(swapChainImageViews[i]);
  }
  device.destroySwapchainKHR(swapChain);
  device.destroyPipelineLayout(pipelineLayout);
  device.destroyRenderPass(renderPass);
  device.destroyCommandPool(commandPool);

  device.destroy();
  removeDebugCallback();
  instance.destroySurfaceKHR(surface);
  instance.destroy();

  glfwDestroyWindow(window);
  glfwTerminate();
}

#include <vku/vku_framework.hpp>
#include <vku/vku.hpp>
#include <framegraph/FG.h>
#include <framework/Window/WindowGLFW.h>
#include <framework/Vulkan/VulkanDeviceExt.h>
#include <pipeline_compiler/VPipelineCompiler.h>
#include <glm/glm.hpp>

const std::string BINARY_DIR = "../../Resources/";

// simple, might not be wholly optimal
std::string loadShaderSrc(const std::string & filename)
{
  auto file = std::ifstream(filename);
  if (file.bad())
  {
    return "";
  }

  return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());  
}

int main() 
{
  // Init GLFW
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  {  
    // Make a window
    std::string title = "Hi, I'm Leonard";
    bool fullScreen = false;
    int width = 800;
    int height = 600;
    GLFWmonitor * monitor = nullptr;
    auto glfwWindow = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);

    vku::Framework fw{ title };
    if (!fw.ok())
    {
      std::cout << "Failed to create Vookoo framework" << std::endl;
      return EXIT_FAILURE;
    }

    // Create device
    vk::Device device = fw.device();

    // Create window to draw into
    vku::Window window{ fw.instance(), device, fw.physicalDevice(), fw.graphicsQueueFamilyIndex(), glfwWindow };
    if (!window.ok())
    {
      std::cout << "Window creation failed" << std::endl;
      return EXIT_FAILURE;
    }

    // Create two shaders
    vku::ShaderModule vert{ device, BINARY_DIR + "helloTriangle.vert.spv" };
    vku::ShaderModule frag{ device, BINARY_DIR + "helloTriangle.frag.spv" };

    // Default pipeline layout
    vku::PipelineLayoutMaker plm{};
    auto pipelineLayout = plm.createUnique(device);

    struct Vertex { 
      glm::vec2 pos; 
      glm::vec3 colour; 
    };

    const std::vector<Vertex> vertices = {
      {{0.f, -0.5f}, {1.f, 0.f, 0.f}},
      {{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
      {{-0.5f, 0.5f},{0.f, 0.f, 1.f}}
    };

    vku::HostVertexBuffer buffer(device, fw.memprops(), vertices);

    // Make pipeline
    vku::PipelineMaker pm{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
    pm.shader(vk::ShaderStageFlagBits::eVertex, vert);
    pm.shader(vk::ShaderStageFlagBits::eFragment, frag);
    pm.vertexBinding(0, sizeof(Vertex));
    pm.vertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos));
    pm.vertexAttribute(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, colour));

    // Create pipeline using renderpass built from window
    auto renderpass = window.renderPass();
    auto &cache = fw.pipelineCache();
    auto pipeline = pm.createUnique(device, cache, *pipelineLayout, renderpass);

    // Create command buffer
    window.setStaticCommands(
      [&pipeline, &buffer](vk::CommandBuffer cb, int imageIndex, vk::RenderPassBeginInfo & rpbi)
      {
        vk::CommandBufferBeginInfo bi{};
        cb.begin(bi);
        cb.beginRenderPass(rpbi, vk::SubpassContents::eInline);
        cb.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
        cb.bindVertexBuffers(0, buffer.buffer(), vk::DeviceSize(0));
        cb.draw(3, 1, 0, 0);
        cb.endRenderPass();
        cb.end();
      }
    );

    // Loop while waiting for window to close
    while (!glfwWindowShouldClose(glfwWindow))
    {
      glfwPollEvents();

      // Draw one triangle
      window.draw(device, fw.graphicsQueue());

      std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ew
    }

    device.waitIdle();
    glfwDestroyWindow(glfwWindow);
  }

  {
    std::unique_ptr<FGC::IWindow> window = std::make_unique<FGC::WindowGLFW>();
    FGC::VulkanDeviceExt vulkan;

    window->Create({ 800, 600 }, "Hi, I'm Leonard");

    vulkan.Create(window->GetVulkanSurface(), "Leonard", "Leonard", VK_API_VERSION_1_1
      , "",
      {{ VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_SPARSE_BINDING_BIT | FG::VK_QUEUE_PRESENT_BIT, 0.0f },
       { VK_QUEUE_COMPUTE_BIT,  0.0f },
       { VK_QUEUE_TRANSFER_BIT, 0.0f }},
      FG::VulkanDevice::GetRecomendedInstanceLayers(),
      FG::VulkanDevice::GetRecomendedInstanceExtensions(),
      FG::VulkanDevice::GetAllDeviceExtensions()
    );

    vulkan.CreateDebugUtilsCallback(FGC::DebugUtilsMessageSeverity_All);

    // Setup device desc
    FG::VulkanDeviceInfo vulkanInfo;
    vulkanInfo.instance = FGC::BitCast<FG::InstanceVk_t>(vulkan.GetVkInstance());
    vulkanInfo.physicalDevice = FGC::BitCast<FG::PhysicalDeviceVk_t>(vulkan.GetVkPhysicalDevice());
    vulkanInfo.device = FGC::BitCast<FG::DeviceVk_t>(vulkan.GetVkDevice());

    for (auto & q : vulkan.GetVkQueues())
    {
      FG::VulkanDeviceInfo::QueueInfo queue;
      queue.handle = FGC::BitCast<FG::QueueVk_t>(q.handle);
      queue.familyFlags = FGC::BitCast<FG::QueueFlagsVk_t>(q.flags);
      queue.familyIndex = q.familyIndex;
      queue.priority = q.priority;

      vulkanInfo.queues.push_back(queue);
    }

    FG::VulkanSwapchainCreateInfo swapchainInfo;
    swapchainInfo.surface = FGC::BitCast<FG::SurfaceVk_t>(vulkan.GetVkSurface());
    swapchainInfo.surfaceSize = window->GetSize();

    FG::FrameGraph frameGraph = FG::IFrameGraph::CreateFrameGraph(vulkanInfo);
    FG::SwapchainID swapchain = frameGraph->CreateSwapchain(swapchainInfo, FG::Default, "Window");

    std::shared_ptr<FG::VPipelineCompiler> compiler = std::make_shared<FG::VPipelineCompiler>(vulkanInfo.physicalDevice, vulkanInfo.device);
    compiler->SetCompilationFlags(FG::EShaderCompilationFlags::AutoMapLocations);

    frameGraph->AddPipelineCompiler(compiler);

    FG::GraphicsPipelineDesc ppln;

    FGC::String vertSrc, fragSrc;
    vertSrc = loadShaderSrc(BINARY_DIR + "helloTriangle.vert");
    fragSrc = loadShaderSrc(BINARY_DIR + "helloTriangle.frag");
    ppln.AddShader(FG::EShader::Vertex, FG::EShaderLangFormat::GLSL_450, "main", vertSrc.c_str());
    ppln.AddShader(FG::EShader::Vertex, FG::EShaderLangFormat::GLSL_450, "main", fragSrc.c_str());


  }

  glfwTerminate();

  return EXIT_SUCCESS;
}
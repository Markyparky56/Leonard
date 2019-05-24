#include <vku/vku_framework.hpp>
#include <vku/vku.hpp>
#include <glm/glm.hpp>

const std::string BINARY_DIR = "../../Resources/";

int main() 
{
  // Init GLFW
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // Make a window
  std::string title = "Hi, I'm Leonard";
  bool fullScreen = false;
  int width = 800;
  int height = 600;
  GLFWmonitor * monitor = nullptr;
  auto glfwWindow = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);

  {
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
  }
  glfwDestroyWindow(glfwWindow);
  glfwTerminate();

  return EXIT_SUCCESS;
}
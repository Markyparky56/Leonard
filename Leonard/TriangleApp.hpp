#pragma once
#include "AppBase.hpp"
#include "EASTL\string.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.hpp>

class TriangleApp : public AppBase
{
public:
  TriangleApp()
    : properties({800, 600, false, "TriangleApp"})
  {
  }

  virtual bool Init() override;
  virtual void Exit() override;
  virtual bool Load() override;
  virtual void Unload() override;
  virtual void Update(float dt) override;
  virtual void Draw() override;

  struct Properties : public PropertiesBase
  {
    eastl::string name;
  } properties;

private:
  GLFWwindow * window;
  vk::Device device;

  vk::UniquePipelineLayout pipelineLayout;
  vk::UniqueRenderPass renderpass;
  vk::UniquePipelineCache pipelineCache;
  vk::UniquePipeline pipeline;
  
  vku::ShaderModule vert, frag;
  vku::VertexBuffer vertBuffer;

};

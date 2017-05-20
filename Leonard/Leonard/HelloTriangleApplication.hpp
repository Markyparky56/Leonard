#pragma once
#include <vulkan/vulkan.hpp>
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
  void createInstance();

  void mainLoop();

  void cleanup();

  // GLFW stuff
  const int WindowWidth = 800, WindowHeight = 600;
  GLFWwindow *window;

  // Vulkan stuff
  vk::Instance instance;
};

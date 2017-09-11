#include "HelloTriangleApplication.hpp"

#include <iostream>
#include <stdexcept>

int main()
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch (UnrecoverableRuntimeException const &e)
  {
    std::cerr << "Unrecoverable Exception!\n"
      << "What: " << e.what() << "\n" 
      << e.message()
      << "\nFile: " << e.fileName() 
      << "\nFunction: " << e.funcName() 
      << "\nLine: " << e.lineNumber() << std::endl;    
    return EXIT_FAILURE;
  }
  catch (UnrecoverableVulkanException const &e)
  {
    std::cerr << "Unrecoverable Exception!\n"
      << "What: " << e.what() << "\n"
      << "Code: " << e.code() << "\n"
      << e.message()
      << "\nFile: " << e.fileName()
      << "\nFunction: " << e.funcName()
      << "\nLine: " << e.lineNumber() << std::endl;
    return EXIT_FAILURE;
  }
  catch (std::runtime_error const &e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

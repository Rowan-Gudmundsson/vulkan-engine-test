#include "triangle.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

#include <iostream>

int main() {
  Triangle triangle;

  try {
    triangle.Run();
  } catch (const std::exception& e) {
    std::cout << "Error occurred: " << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cout << "Something else was thrown." << std::endl;
  }

  return EXIT_SUCCESS;
}

PROJECT(Vulkan)
CMAKE_MINIMUM_REQUIRED(VERSION 3.7)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/CMakeModules")
add_definitions( -DMAGICKCORE_QUANTUM_DEPTH=16 )
add_definitions( -DMAGICKCORE_HDRI_ENABLE=0 )

FIND_PACKAGE(vulkan REQUIRED)
FIND_PACKAGE(glfw3 REQUIRED)
FIND_PACKAGE(GLM REQUIRED)
FIND_PACKAGE(Bullet REQUIRED)
FIND_PACKAGE(ASSIMP REQUIRED)
# FIND_PACKAGE(ImageMagick COMPONENTS Magick++ REQUIRED )

SET(CXX11_FLAGS "-std=c++17 -lassimp")
SET(CDEBUG_FLAGS -g)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS} ${CDEBUG_FLAGS}")
SET(TARGET_LIBRARIES "${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${ASSIMP_LIBRARIES}")

# Find where Magick++-config lives

IF(UNIX)
  ADD_DEFINITIONS(-DUNIX)
ENDIF(UNIX)


INCLUDE_DIRECTORIES(
  "${PROJECT_SOURCE_DIR}/include"
  ${SDL2_INCLUDE_DIR}
  ${GLM_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS}
  ${ImageMagick_INCLUDE_DIRS}
  ${BULLET_INCLUDE_DIRS}
)

# Copy shaders, models, and default config
# FILE(COPY src/shaders DESTINATION .)
# FILE(COPY models DESTINATION .)
# FILE(COPY textures DESTINATION .)
# FILE(COPY config.json DESTINATION .)


# Set Includes
SET(INCLUDES ${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${INCLUDES} ${ASSIMP_INCLUDE_DIRS} ${ImageMagick_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS})

# Set sources
FILE(GLOB_RECURSE SOURCES "src/*.cpp")
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCES})

add_custom_target("${PROJECT_NAME}_SUCCESSFUL" ALL
                  DEPENDS ${PROJECT_NAME}
                  COMMAND ${CMAKE_COMMAND} -E echo ""
                  COMMAND ${CMAKE_COMMAND} -E echo "====================="
                  COMMAND ${CMAKE_COMMAND} -E echo "  Compile complete!"
                  COMMAND ${CMAKE_COMMAND} -E echo "====================="
                  COMMAND ${CMAKE_COMMAND} -E echo "${CMAKE_CURRENT_BINARY_DIR}"
                 )

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENGL_LIBRARY} ${SDL2_LIBRARY} ${ASSIMP_LIBRARY} ${ImageMagick_LIBRARIES} ${BULLET_LIBRARIES} glfw Vulkan::Vulkan)
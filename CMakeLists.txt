CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

PROJECT (vulkan-testings VERSION "0.1")

SET (ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)

# ╔═════════════════════════╗
# ║ C++ Compilation options ║
# ╚═════════════════════════╝

SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_FLAGS "-Wall -pedantic -fno-exceptions -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi")
SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -DDEBUG")
SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -fopenmp -DNDEBUG")

# ╔═════════════╗
# ║ Vim options ║
# ╚═════════════╝

SET (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ╔═════════╗
# ║ Sources ║
# ╚═════════╝

ADD_EXECUTABLE (${PROJECT_NAME}
    src/main.cpp

    # Core
    src/Core/types.h
    src/Core/utils.h
    src/Core/Core.h
    src/Core/Core.cpp

    # ECS Subsytem
    src/Core/Subsystems/ECS/EntityManager.h
    src/Core/Subsystems/ECS/ComponentArray.h
    src/Core/Subsystems/ECS/ComponentManager.h
    src/Core/Subsystems/ECS/System.h
    src/Core/Subsystems/ECS/SystemManager.h
    src/Core/Subsystems/ECS/ECSManager.h
    
    # Window Subsystem
    src/Core/Subsystems/Window/Window.h
    src/Core/Subsystems/Window/Window.cpp

    # Renderer Subsystem
    src/Core/Subsystems/Renderer/Renderer.h
    src/Core/Subsystems/Renderer/Renderer.cpp
    src/Core/Subsystems/Renderer/Shader.h
    src/Core/Subsystems/Renderer/Shader.cpp

    # Renderer World
    src/Core/Subsystems/Renderer/world/World.h
    src/Core/Subsystems/Renderer/world/World.cpp
    src/Core/Subsystems/Renderer/world/Scene.h
    src/Core/Subsystems/Renderer/world/Scene.cpp
    src/Core/Subsystems/Renderer/world/Node.h
    src/Core/Subsystems/Renderer/world/Node.cpp
    src/Core/Subsystems/Renderer/world/Mesh.h
    src/Core/Subsystems/Renderer/world/Mesh.cpp
    src/Core/Subsystems/Renderer/world/Texture.h
    src/Core/Subsystems/Renderer/world/Texture.cpp

    # Renderer tempname
    src/Core/Subsystems/Renderer/tempname/Swapchain.h
    src/Core/Subsystems/Renderer/tempname/Swapchain.cpp
    src/Core/Subsystems/Renderer/tempname/DescriptorPool.h
    src/Core/Subsystems/Renderer/tempname/DescriptorPool.cpp
    src/Core/Subsystems/Renderer/tempname/DescriptorSet.h
    src/Core/Subsystems/Renderer/tempname/DescriptorSet.cpp
    src/Core/Subsystems/Renderer/tempname/GraphicsPipeline.h
    src/Core/Subsystems/Renderer/tempname/GraphicsPipeline.cpp

    # Input Subsystem
    src/Core/Subsystems/Input/InputManager.h
    src/Core/Subsystems/Input/InputManager.cpp

    # Components
    src/Components/Transform.h
    src/Components/Camera.h

    # Systems
    src/Systems/CameraHandler.h
    src/Systems/CameraHandler.cpp
)

# Shaderc
TARGET_LINK_LIBRARIES(${PROJECT_NAME} libshaderc_combined.a)

# tinygltf
TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} SYSTEM PRIVATE "extern/tinygltf")


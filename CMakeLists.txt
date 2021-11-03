CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

PROJECT (vulkan-testings VERSION "0.1")

SET (ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)

# ╔═════════════════════════╗
# ║ C++ Compilation options ║
# ╚═════════════════════════╝

SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_FLAGS "-std=c++20 -Wall -pedantic -fno-exceptions -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi")
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
    src/Core/Subsystems/Window/WindowManager.h
    src/Core/Subsystems/Window/WindowManager.cpp

    # Renderer Subsystem
    src/Core/Subsystems/Renderer/RendererManager.h
    src/Core/Subsystems/Renderer/RendererManager.cpp
    src/Core/Subsystems/Renderer/Shader.h
    src/Core/Subsystems/Renderer/Shader.cpp
    src/Core/Subsystems/Renderer/Swapchain.h
    src/Core/Subsystems/Renderer/Swapchain.cpp

    # Components
    src/Components/Gravity.h
    src/Components/RigidBody.h
    src/Components/Transform.h

    # Systems
    src/Systems/Physics.h
    src/Systems/Physics.cpp
)

# Shaderc
TARGET_LINK_LIBRARIES(${PROJECT_NAME} libshaderc_combined.a)

# STB
TARGET_INCLUDE_DIRECTORIES (${PROJECT_NAME} SYSTEM PRIVATE "extern/stb")


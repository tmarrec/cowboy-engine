CMAKE_MINIMUM_REQUIRED (VERSION 3.8)

PROJECT (vulkan-testings VERSION "0.1")

SET (ALLOW_DUPLICATE_CUSTOM_TARGETS TRUE)

# ╔═════════════════════════╗
# ║ C++ Compilation options ║
# ╚═════════════════════════╝

SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_FLAGS "-std=c++20 -Wall -pedantic -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi")
SET (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -DDEBUG -lgomp -fopenmp")
SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -lgomp -fopenmp -DNDEBUG")

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
    src/Core/EntityManager.h
    src/Core/ComponentArray.h
    src/Core/ComponentManager.h
    src/Core/System.h
    src/Core/SystemManager.h
    src/Core/Coordinator.h
    src/Core/Window.h
    src/Core/Window.cpp

    # Components
    src/Components/Gravity.h
    src/Components/RigidBody.h
    src/Components/Transform.h

    # Systems
    src/Systems/Physics.h
    src/Systems/Physics.cpp
)


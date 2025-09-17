cmake_minimum_required(VERSION 3.16)
project(titanium-gpu-physics VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find optional packages
find_package(Vulkan)

# Check if Vulkan is available
if(Vulkan_FOUND)
    message(STATUS "Vulkan found - GPU physics will be available")
    add_compile_definitions(VULKAN_AVAILABLE)
    set(VULKAN_SOURCES
        src/PhysicsEngine/GPUPhysicsEngine/GPUPhysicsEngine.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/VulkanInstance.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/VulkanDevice.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/VulkanCommandPool.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/VulkanContext.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/physics/BufferManager.cpp
        src/PhysicsEngine/GPUPhysicsEngine/components/vulkan/physics/ComputePipeline.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/vulkanmanager/VulkanManager.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/physicsmanager/GPUPhysicsManager.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/particlemanager/ParticleManager.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/shadermanager/ShaderManager.cpp
    # GPU collision manager removed; GPU handles particles only
    )
    set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
    set(VULKAN_LIBRARIES ${Vulkan_LIBRARIES})
else()
    message(STATUS "Vulkan not found - building CPU-only physics system")
endif()

# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

# Add executable with ECS-only CPU physics and optional GPU particle support
add_executable(titanium-gpu-physics
    src/main.cpp
    # Core physics engine
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/components/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/entities/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    # Managers (CPU-only compatible)
    src/PhysicsEngine/managers/logmanager/Logger.cpp
    # Optional GPU sources
    ${VULKAN_SOURCES}
)

# Include directories
target_include_directories(titanium-gpu-physics PRIVATE
    src
    ${VULKAN_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(titanium-gpu-physics
    ${VULKAN_LIBRARIES}
)

# Compile shaders when Vulkan is available
if(Vulkan_FOUND)
    find_program(GLSLANGVALIDATOR glslangValidator REQUIRED)

    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/PhysicsEngine/GPUPhysicsEngine/components/shaders)
    set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)

    file(GLOB SHADERS
        ${SHADER_SOURCE_DIR}/*.comp
        ${SHADER_SOURCE_DIR}/*.vert
        ${SHADER_SOURCE_DIR}/*.frag
    )

    foreach(SHADER ${SHADERS})
        get_filename_component(FILENAME ${SHADER} NAME)
        set(SPV "${SHADER_BINARY_DIR}/${FILENAME}.spv")
        add_custom_command(
            OUTPUT ${SPV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BINARY_DIR}"
            COMMAND ${GLSLANGVALIDATOR} -V ${SHADER} -o ${SPV}
            DEPENDS ${SHADER}
        )
        list(APPEND SPV_SHADERS ${SPV})
    endforeach()

    add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
    add_dependencies(titanium-gpu-physics shaders)
endif()

# Set output directory for shaders
set_target_properties(titanium-gpu-physics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Keep CTest enabled even if no test targets are currently defined
enable_testing()
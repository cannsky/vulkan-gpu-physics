cmake_minimum_required(VERSION 3.16)
project(tulpar-physics VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Vulkan REQUIRED)

# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

# Add executable
add_executable(tulpar-physics
    src/main.cpp
    src/vulkan/VulkanInstance.cpp
    src/vulkan/VulkanDevice.cpp
    src/vulkan/VulkanCommandPool.cpp
    src/vulkan/VulkanContext.cpp
    src/components/physics/PhysicsEngine.cpp
    src/vulkan/physics/BufferManager.cpp
    src/vulkan/physics/ComputePipeline.cpp
    src/components/rigidbody/RigidBodySystem.cpp
    src/managers/logmanager/Logger.cpp
    src/managers/vulkanmanager/VulkanManager.cpp
    src/managers/physicsmanager/PhysicsManager.cpp
    src/managers/particlemanager/ParticleManager.cpp
    src/managers/collisionmanager/CollisionManager.cpp
    src/managers/collisionmanager/workers/BroadPhaseWorker.cpp
    src/managers/collisionmanager/workers/DetectCollisionWorker.cpp
    src/managers/collisionmanager/workers/ContactResolverWorker.cpp
    src/managers/collisionmanager/workers/GPUBufferWorker.cpp
    src/managers/shadermanager/ShaderManager.cpp
    src/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/factories/RigidBodyFactory.cpp
    src/managers/testmanager/TestManager.cpp
)

# Include directories
target_include_directories(tulpar-physics PRIVATE
    src
    ${Vulkan_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(tulpar-physics
    ${Vulkan_LIBRARIES}
)

# Compile shaders
find_program(GLSLANGVALIDATOR glslangValidator REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/components/shaders)
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
add_dependencies(tulpar-physics shaders)

# Set output directory for shaders
set_target_properties(tulpar-physics PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Enable testing
enable_testing()

# Add test executable with new framework
add_executable(physics-tests
    src/test_runner.cpp
    src/managers/testmanager/TestManager.cpp
    src/components/rigidbody/RigidBodySystem.cpp
    src/managers/collisionmanager/workers/BroadPhaseWorker.cpp
    src/managers/collisionmanager/workers/DetectCollisionWorker.cpp
    src/managers/collisionmanager/workers/ContactResolverWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(physics-tests PRIVATE
    src
    src/managers/testmanager/tests
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(physics-tests
    ${Vulkan_LIBRARIES}
)

# Register tests with CTest
add_test(NAME PhysicsEngineTests COMMAND physics-tests)

# Add legacy test executable for compatibility
add_executable(legacy-physics-tests
    src/test_physics_systems.cpp
    src/managers/testmanager/TestManager.cpp
    src/components/rigidbody/RigidBodySystem.cpp
    src/managers/collisionmanager/workers/BroadPhaseWorker.cpp
    src/managers/collisionmanager/workers/DetectCollisionWorker.cpp
    src/managers/collisionmanager/workers/ContactResolverWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(legacy-physics-tests PRIVATE
    src
    src/managers/testmanager/tests
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(legacy-physics-tests
    ${Vulkan_LIBRARIES}
)

# Register legacy tests with CTest
add_test(NAME LegacyPhysicsTests COMMAND legacy-physics-tests)
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
        src/gpu_physics/GPUPhysicsEngine.cpp
        src/gpu_physics/components/vulkan/VulkanInstance.cpp
        src/gpu_physics/components/vulkan/VulkanDevice.cpp
        src/gpu_physics/components/vulkan/VulkanCommandPool.cpp
        src/gpu_physics/components/vulkan/VulkanContext.cpp
        src/gpu_physics/components/vulkan/physics/BufferManager.cpp
        src/gpu_physics/components/vulkan/physics/ComputePipeline.cpp
        src/gpu_physics/managers/vulkanmanager/VulkanManager.cpp
        src/gpu_physics/managers/physicsmanager/GPUPhysicsManager.cpp
        src/gpu_physics/managers/particlemanager/ParticleManager.cpp
        src/gpu_physics/managers/shadermanager/ShaderManager.cpp
        src/gpu_physics/managers/collisionmanager/CollisionManager.cpp
        src/gpu_physics/managers/collisionmanager/workers/BroadPhaseWorker.cpp
        src/gpu_physics/managers/collisionmanager/workers/DetectCollisionWorker.cpp
        src/gpu_physics/managers/collisionmanager/workers/ContactResolverWorker.cpp
    )
    set(VULKAN_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS})
    set(VULKAN_LIBRARIES ${Vulkan_LIBRARIES})
else()
    message(STATUS "Vulkan not found - building CPU-only physics system")
    set(VULKAN_SOURCES "")
    set(VULKAN_INCLUDE_DIRS "")
    set(VULKAN_LIBRARIES "")
endif()

# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/shaders)

# Add executable with new titanium-physics hybrid architecture
add_executable(titanium-gpu-physics
    src/main.cpp
    # New hybrid physics system
    src/physics_engine.cpp
    src/cpu_physics/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/cpu_physics/managers/ecs_manager/ecs_manager.cpp
    src/cpu_physics/factories/rigidbody_component_factory.cpp
    src/cpu_physics/factories/rigidbody_entity_factory.cpp
    src/cpu_physics/systems/cpu_physics_collision_system.cpp
    # Physics Managers
    src/cpu_physics/managers/physicsmanager/CPUPhysicsManager.cpp
    src/cpu_physics/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/cpu_physics/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    ${VULKAN_SOURCES}
    # Managers (CPU-only compatible)
    src/managers/logmanager/Logger.cpp
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

# Compile shaders (only if Vulkan is available)
if(Vulkan_FOUND)
    find_program(GLSLANGVALIDATOR glslangValidator REQUIRED)

    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/gpu_physics/components/shaders)
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

# Enable testing
enable_testing()

# Add test executable with new consolidated framework  
add_executable(physics-tests
    src/tests/test.cpp
    src/components/tests/TestManager.cpp
    src/cpu_physics/rigidbody/RigidBodySystem.cpp
    # New hybrid physics system
    src/physics_engine.cpp
    src/cpu_physics/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/cpu_physics/managers/ecs_manager/ecs_manager.cpp
    src/cpu_physics/factories/rigidbody_component_factory.cpp
    src/cpu_physics/factories/rigidbody_entity_factory.cpp
    src/cpu_physics/systems/cpu_physics_collision_system.cpp
    ${VULKAN_SOURCES}
    # CPU Physics Workers
    src/cpu_physics/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/cpu_physics/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(physics-tests PRIVATE
    src
    src/components/tests/tests
    ${VULKAN_INCLUDE_DIRS}
)

target_link_libraries(physics-tests
    ${VULKAN_LIBRARIES}
)

# Register tests with CTest
add_test(NAME PhysicsEngineTests COMMAND physics-tests)

# Add legacy test executable for compatibility
add_executable(legacy-physics-tests
    src/tests/test.cpp
    src/components/tests/TestManager.cpp
    src/cpu_physics/rigidbody/RigidBodySystem.cpp
    # New hybrid physics system
    src/physics_engine.cpp
    src/cpu_physics/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/cpu_physics/managers/ecs_manager/ecs_manager.cpp
    src/cpu_physics/factories/rigidbody_component_factory.cpp
    src/cpu_physics/factories/rigidbody_entity_factory.cpp
    src/cpu_physics/systems/cpu_physics_collision_system.cpp
    ${VULKAN_SOURCES}
    # CPU Physics Workers
    src/cpu_physics/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/cpu_physics/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(legacy-physics-tests PRIVATE
    src
    src/components/tests/tests
    ${VULKAN_INCLUDE_DIRS}
)

target_link_libraries(legacy-physics-tests
    ${VULKAN_LIBRARIES}
)

# Register legacy tests with CTest
add_test(NAME LegacyPhysicsTests COMMAND legacy-physics-tests)

# Add titanium physics standalone test (CPU only)
add_executable(test-titanium-physics
    src/tests/test.cpp
    src/physics_engine.cpp
    src/cpu_physics/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/cpu_physics/managers/ecs_manager/ecs_manager.cpp
    src/cpu_physics/factories/rigidbody_component_factory.cpp
    src/cpu_physics/factories/rigidbody_entity_factory.cpp
    src/cpu_physics/systems/cpu_physics_collision_system.cpp
    # Physics Managers
    src/cpu_physics/managers/physicsmanager/CPUPhysicsManager.cpp
    src/cpu_physics/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/cpu_physics/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(test-titanium-physics PRIVATE
    src
)

# Register titanium physics test with CTest
add_test(NAME TitaniumPhysicsTest COMMAND test-titanium-physics)

# Add CPU-only demo executable
add_executable(titanium-demo-cpu
    src/tests/test.cpp
    src/physics_engine.cpp
    src/cpu_physics/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/cpu_physics/managers/ecs_manager/ecs_manager.cpp
    src/cpu_physics/factories/rigidbody_component_factory.cpp
    src/cpu_physics/factories/rigidbody_entity_factory.cpp
    src/cpu_physics/systems/cpu_physics_collision_system.cpp
    # Physics Managers
    src/cpu_physics/managers/physicsmanager/CPUPhysicsManager.cpp
    src/cpu_physics/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/cpu_physics/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/managers/logmanager/Logger.cpp
)

target_include_directories(titanium-demo-cpu PRIVATE
    src
)

# Register CPU demo test with CTest
add_test(NAME TitaniumCPUDemo COMMAND titanium-demo-cpu)
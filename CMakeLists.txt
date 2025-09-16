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
        src/PhysicsEngine/GPUPhysicsEngine/managers/collisionmanager/CollisionManager.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/collisionmanager/workers/BroadPhaseWorker.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/collisionmanager/workers/DetectCollisionWorker.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/collisionmanager/workers/ContactResolverWorker.cpp
        src/PhysicsEngine/GPUPhysicsEngine/managers/collisionmanager/workers/GPUBufferWorker.cpp
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
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    # Physics Managers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/CPUPhysicsManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    ${VULKAN_SOURCES}
    # Managers (CPU-only compatible)
    src/PhysicsEngine/managers/logmanager/Logger.cpp
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

# Enable testing
enable_testing()

# Add test executable with new consolidated framework  
add_executable(physics-tests
    src/tests/test.cpp
    src/tests/components/tests/TestManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/rigidbody/RigidBodySystem.cpp
    # New hybrid physics system
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    ${VULKAN_SOURCES}
    # CPU Physics Workers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/PhysicsEngine/managers/logmanager/Logger.cpp
)

target_include_directories(physics-tests PRIVATE
    src
    src/tests/components/tests/tests
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
    src/tests/components/tests/TestManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/rigidbody/RigidBodySystem.cpp
    # New hybrid physics system
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    ${VULKAN_SOURCES}
    # CPU Physics Workers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/PhysicsEngine/managers/logmanager/Logger.cpp
)

target_include_directories(legacy-physics-tests PRIVATE
    src
    src/tests/components/tests/tests
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
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    # Physics Managers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/CPUPhysicsManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/PhysicsEngine/managers/logmanager/Logger.cpp
    # GPU Physics sources when Vulkan is available
    ${VULKAN_SOURCES}
)

target_include_directories(test-titanium-physics PRIVATE
    src
    ${VULKAN_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(test-titanium-physics
    ${VULKAN_LIBRARIES}
)

# Register titanium physics test with CTest
add_test(NAME TitaniumPhysicsTest COMMAND test-titanium-physics)

# Add CPU-only demo executable
add_executable(titanium-demo-cpu
    src/tests/test.cpp
    src/PhysicsEngine/PhysicsEngine.cpp
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    # Physics Managers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/CPUPhysicsManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/PhysicsEngine/managers/logmanager/Logger.cpp
    # GPU Physics sources when Vulkan is available
    ${VULKAN_SOURCES}
)

target_include_directories(titanium-demo-cpu PRIVATE
    src
    ${VULKAN_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(titanium-demo-cpu
    ${VULKAN_LIBRARIES}
)

# Register CPU demo test with CTest
add_test(NAME TitaniumCPUDemo COMMAND titanium-demo-cpu)

# Interface demo executable
add_executable(interface-demo
    src/PhysicsEngine/CPUPhysicsEngine/tests/interface_demo.cpp
    # Core physics system
    src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp
    # ECS Architecture
    src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyComponentFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/factories/RigidbodyEntityFactory.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/systems/EnhancedCPUPhysicsCollisionSystem.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteComponents.cpp
    src/PhysicsEngine/CPUPhysicsEngine/concrete/ConcreteEntity.cpp
    # Physics Managers
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/CPUPhysicsManager.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/RigidBodyWorker.cpp
    src/PhysicsEngine/CPUPhysicsEngine/managers/physicsmanager/workers/PhysicsLayerWorker.cpp
    src/PhysicsEngine/managers/logmanager/Logger.cpp
)

target_include_directories(interface-demo PRIVATE
    src
    ${VULKAN_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(interface-demo
    ${VULKAN_LIBRARIES}
)

# Register interface demo test with CTest
add_test(NAME InterfaceDemo COMMAND interface-demo)
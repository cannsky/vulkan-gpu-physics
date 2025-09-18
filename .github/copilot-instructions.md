# Titanium GPU Physics - Copilot Instructions

Titanium GPU Physics is a hybrid GPU/CPU physics simulation system using Vulkan compute shaders and modern C++23. The **Titanium Physics Engine** separates particle/fluid simulations (GPU) from complex rigidbody operations (CPU) using an ECS architecture.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Bootstrap, Build, and Test the Repository

**CRITICAL BUILD TIMING**: 
- **NEVER CANCEL BUILDS OR TESTS** - Use timeout 60+ minutes for builds, 30+ minutes for tests
- Configuration time: ~1.4 seconds
- Full build time: ~7 seconds (CPU+GPU) or ~4 seconds (CPU-only)
- Test compilation time: ~9 seconds
- Test execution time: <1 second

#### Essential Dependencies (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y cmake build-essential
sudo apt install -y vulkan-tools libvulkan-dev vulkan-utility-libraries-dev
sudo apt install -y spirv-tools glslang-tools
```

#### Build Process
```bash
# Clone and navigate to repository root
cd vulkan-gpu-physics

# Create build directory
mkdir build && cd build

# Configure CMake (with Vulkan support)
cmake ..

# Build - NEVER CANCEL: Takes ~7 seconds, use 60+ minute timeout as safety
make -j$(nproc)

# Run the application
./titanium-gpu-physics
```

#### CPU-Only Build (When Vulkan Unavailable)
When Vulkan libraries are not installed or not detected:
```bash
# CMake will automatically detect absence and configure CPU-only build
cmake ..  # Will output "Vulkan not found - building CPU-only physics system"

# Build - NEVER CANCEL: Takes ~4 seconds CPU-only, use 60+ minute timeout as safety  
make -j$(nproc)

# Run CPU-only version
./titanium-gpu-physics  # Will output "Vulkan not compiled - using CPU-only physics"
```

#### Test Execution
```bash
# Compile test executable - NEVER CANCEL: Takes ~9 seconds, use 30+ minute timeout
g++ -std=c++23 -I src src/tests/test.cpp src/PhysicsEngine/PhysicsEngine.cpp src/PhysicsEngine/CPUPhysicsEngine/CPUPhysicsEngine.cpp src/PhysicsEngine/CPUPhysicsEngine/managers/ECSManager/ECSManager.cpp src/PhysicsEngine/CPUPhysicsEngine/factories/components/RigidbodyComponentFactory.cpp src/PhysicsEngine/CPUPhysicsEngine/factories/entities/RigidbodyEntityFactory.cpp src/PhysicsEngine/CPUPhysicsEngine/systems/CpuPhysicsCollisionSystem.cpp src/PhysicsEngine/CPUPhysicsEngine/systems/BaseCPUPhysicsSystem.cpp src/PhysicsEngine/managers/logmanager/Logger.cpp -o test-titanium-physics

# Run tests - NEVER CANCEL: Takes <1 second, use 30+ minute timeout as safety
./test-titanium-physics
```

**Expected Test Output**: 6 tests, 100% pass rate, "🎉 All tests passed!"

## Validation

### Manual Validation Requirements
**ALWAYS run through complete validation scenarios after making changes:**

#### Scenario 1: CPU-Only Physics Validation
```bash
# Remove Vulkan to test CPU-only mode (optional)
sudo apt remove -y vulkan-tools libvulkan-dev vulkan-utility-libraries-dev

# Build and run CPU-only version
mkdir build && cd build
cmake ..  # Should output "Vulkan not found"
make -j$(nproc)
./titanium-gpu-physics

# Expected: "Vulkan not compiled - using CPU-only physics"
# Expected: Physics simulation runs with ~63 FPS, falling objects
# Expected: No Vulkan errors, only CPU rigidbody physics
```

#### Scenario 2: Hybrid GPU/CPU Physics Validation
```bash
# Ensure Vulkan is installed
sudo apt install -y vulkan-tools libvulkan-dev vulkan-utility-libraries-dev spirv-tools glslang-tools

# Build and run hybrid version
rm -rf build && mkdir build && cd build
cmake ..  # Should output "Vulkan found - GPU physics will be available"
make -j$(nproc)  # Should compile shaders: "Generating shaders/particle_physics.comp.spv"
./titanium-gpu-physics

# Expected outcomes:
# - If Vulkan drivers available: GPU particle physics enabled
# - If no GPU/drivers: "Failed to create Vulkan instance!" then CPU fallback
# - Physics simulation runs with ~63 FPS, falling objects
```

#### Scenario 3: Test Suite Validation
```bash
# Test CPU-only mode
g++ -std=c++23 -I src src/tests/test.cpp [source files...] -o test-titanium-physics
./test-titanium-physics

# Expected: All 6 tests pass (CPU Physics, RigidBody creation, simulation step, etc.)
# Expected: "🎉 All tests passed!" with 100% success rate
```

### Build and Application Behavior
- **Successful build**: Application compiles and links without errors
- **Successful execution**: Physics simulation runs smoothly at ~63 FPS
- **GPU mode**: Shaders compile during build, Vulkan context initializes
- **CPU fallback**: Graceful degradation when Vulkan unavailable
- **Test validation**: 100% test pass rate with comprehensive physics system tests

## Common Tasks

### Key Projects in This Codebase

#### Core Physics Engine (`src/PhysicsEngine/`)
- **PhysicsEngine.cpp/h**: Main hybrid physics system coordinator
- **CPUPhysicsEngine/**: ECS-based CPU rigidbody physics system
- **GPUPhysicsEngine/**: Vulkan compute-based particle physics system

#### ECS Architecture (`src/PhysicsEngine/CPUPhysicsEngine/`)
- **managers/ECSManager/**: Entity-Component-System management
- **systems/**: Physics systems (collision, dynamics)
- **factories/**: Component and entity creation
- **components/**: Physics components (RigidBody, Transform, etc.)

#### Vulkan Components (`src/PhysicsEngine/GPUPhysicsEngine/`)
- **components/vulkan/**: Core Vulkan abstraction (Instance, Device, Context)
- **components/shaders/**: GLSL compute shaders for physics
- **managers/**: Vulkan resource managers (Shader, Particle, Buffer)

#### Test System (`src/tests/`)
- **test.cpp**: Simple unified test framework with 6 essential tests
- **components/tests/**: Advanced test infrastructure (TestManager, etc.)

### Frequently Used Commands Output Reference

#### Repository Root Structure
```
.github/             # GitHub workflows and configurations
CMakeLists.txt       # Main build configuration
README.md           # Project documentation
docs/               # Detailed component documentation  
src/                # Source code
├── PhysicsEngine/  # Core physics systems
├── main.cpp        # Application entry point
└── tests/          # Test infrastructure
```

#### Build Artifacts (build/)
```
titanium-gpu-physics    # Main executable
shaders/               # Compiled SPIR-V shaders (GPU mode only)
*.spv                  # Shader bytecode files
```

#### CI/CD Integration
- **GitHub Actions**: `.github/workflows/vulkan-ubuntu.yml` (Vulkan+Ubuntu)
- **Multi-platform**: `.github/workflows/cmake-multi-platform.yml` (Windows/Linux)
- **Dependencies**: Auto-installs Vulkan SDK, builds both CPU and GPU modes

### Development Workflow
1. **ALWAYS** build and test before making changes to understand current state
2. **ALWAYS** run full validation scenarios after changes
3. **NEVER CANCEL** builds or tests - they complete quickly but use generous timeouts
4. **ALWAYS** test both CPU-only and hybrid GPU/CPU modes when modifying core systems
5. **ALWAYS** run the test suite after physics engine modifications

### Architecture Notes
- **CPU Physics**: Handles complex rigidbody dynamics, collision detection, ECS management
- **GPU Physics**: Handles massive particle simulations via Vulkan compute shaders
- **Hybrid Mode**: Seamlessly combines CPU rigidbodies with GPU particles
- **Graceful Degradation**: Automatic fallback to CPU-only when GPU unavailable
- **Modern C++23**: Requires compatible compiler, uses latest language features

### Performance Expectations
- **Build Time**: 4-7 seconds (much faster than typical projects)
- **Startup Time**: Instant for CPU-only, <1 second for GPU initialization
- **Runtime Performance**: 60+ FPS physics simulation with 10 rigidbodies
- **Memory Usage**: Efficient ECS architecture, proper Vulkan memory management

### Troubleshooting Common Issues
- **"Vulkan not found"**: Install Vulkan development packages
- **"Failed to create Vulkan instance"**: GPU drivers issue, will fallback to CPU
- **"glslangValidator not found"**: Install glslang-tools package
- **Test failures**: Usually indicates core physics logic regression
- **Build errors**: Check C++23 compiler compatibility and dependencies
# v4d_core

Source files for **Vulkan4D Core Library** (Core Utilities and Helper functions)

These source files compile into `v4d.dll` (or `v4d.so` under Linux)

It must be linked with when building a game using the Vulkan4D Engine.

It may also be linked to modules when needed


## V4D Core Utilities

Core Utilities are a solid part of Vulkan4D's Core to provide developers with interfaces to the Hardware. 

They are split into 7 categories (`Audio`, `Graphics`, `Crypto`, `Data`, `IO`, `Networking`, `Processing`)

## V4D Modules (modding system)

Vulkan4D is modular and built from the ground up to fully support Modding. 

A module may contain resources and shared libraries that are loaded at runtime into the application.

See modding documentation in [the sample module repository](https://github.com/Vulkan4D/v4d_module_sample)

## File Structure
The core consists of the following structure :
- `helpers/*` Contains header-only `.hpp` files with helper methods
- `utilities/*` Contains subdirectories for utility categories with all their utilities
- `Core.h/cpp` Core source compiled only in the core library
- `v4d.h` Main Header file to be included in anything that is part of V4D
- `V4D_Mod.h/cpp` Modding interface base class
- `tests.cxx` Core Unit Tests
- `README.md` this documentation
- `*.hh` Grouped Header files included in v4d.h


## Coding Standards

### Formatting
- `{}` brackets start on the SAME LINE, preceded by a space
    ```c++
    int main(int argc, char** args) {
        std::cout << "Hello V4D !" << std::endl;
    }
    ```
- Methods and conditions on multiple lines are all or nothing (all in one line, or one line per argument)
- There is no limit on horizontal scrolling, as long as it is easy to read
- Space after keywords (`if`, `for`, `while`,...)
- Space before and After Stream operators (`<<` `>>`)
- Indentations are tabs, not spaces
- Comments symbols `//` are between spaces
- Assignments and comparison operators (`=` and `==`) have spaces before and after
- All math and binary operators (`+`, `-`, `*`, /, `&`, `|`,...) are between spaces and not stuck on variables or numbers
    - `int b = a + 5;` GOOD
    - `int b=a+5;` BAD
- Pointers and references (where to place the `*` and `&` symbols) : 
    - Declarations have the symbol sticking on the type
        - Pointer declarations : `int* ptr;`
        - Pointer type arguments in methods : `void func(int* arg) {`
        - Argument by reference in methods : `void func(int& arg) {`
    - Dereferencing and getting the address have the symbol sticking on the variable name
        - Assigning the referenced value of a pointer variable : `int val = *prt;`
        - Assigning a variable by reference : `int var2 = &var1;`
        - Assigning a value to a dereferenced pointer : `*ptr = 5;`

### Rules and modern conventions
- variables initialized using braces `{ }`
- use `static_cast<>` instead of c-style casting
- use verbose types like `uint32_t` instead of `unsigned int`
- use `using` isntead of `typedef`
- don't copy-paste code from stack-overflow
- don't write code that you don't understand

### Naming conventions
- namespaces are camelCase and follow directory names/hierarchy
- class file names are PascalCase
- class names are PascalCase
- method names are PascalCase
- class members and variables are camelCase
- const variables and macros are ALL_CAPS
- method arguments are camelCase with the same name as an underlying class member that is directly set
- typedefs are all_lowercase
- file names unrelated to a class are all_lowercase
- Unsafe/temporary method/member names are surrounded by underscores : \_UnsafeMethod\_()
- Public method/member names intended only for internal use should start with two underscores : __V4D_SomeMethod()

### File Extensions
- `.cpp` C++ Source Files compiled via g++, must have a matching .h (except main file)
- `.hpp` C++ Header-only source files (no matching .cpp)
- `.h` C++ Header files included from a matching .cpp source file
- `.cxx` C++ source files Reserved for Unit Tests *(Subject to change)*
- `.hh` C++ header-only files without any associated code (config files, enum, ...)

### Critical method names for correct memory management
- `Create*` must be followed by `Destroy*`
- `New*` must be followed by `Delete*`
- `Allocate*` must be followed by `Free*`
- `Begin*` must be followed by `End*`
- `Start*` must be followed by `Stop*` or `Restart*`
- `Load*` must be followed by `Unload*` or `Reload*` (`Load` should not be called more than once, It is preferable to call `Reload` over `Unload`+`Load`)
- `Init*` must only be called once per instance, and no need to free any memory
- `Configure*`, `Generate*`, `Make*`, `Build*`, `Read*`, `Write*` may be called any number of times, and no need to free any memory


## Application Method/Modules execution order

#### Start
- V4D_Mod:: `ModuleLoad` ()
- V4D_Mod:: `OrderIndex` ()
- Instantiate Vulkan
- Create Window
- Create Renderer
- Create Scene
- V4D_Mod:: `LoadScene` ()
- V4D_Mod:: `InitWindow` ()
- V4D_Mod:: `InputCallbackName` ()
- Add Input Callbacks
- V4D_Mod:: `InitRenderer` ()
- V4D_Mod:: `InitVulkanLayouts` ()
- V4D_Mod:: `ConfigureShaders` ()
- V4D_Mod:: `ReadShaders` ()
- V4D_Mod:: `ScorePhysicalDeviceSelection` ()
- V4D_Mod:: `InitVulkanDeviceFeatures` ()
- V4D_Mod:: `InitRenderingDevice` ()
- Create Rendering Device
- Create Allocator
- V4D_Mod:: `ConfigureRenderer` ()
- V4D_Mod:: `CreateVulkanSyncObjects` ()
- Create SwapChain
- Create Command Pools
- V4D_Mod:: `AllocateVulkanBuffers` ()
- V4D_Mod:: `CreateVulkanResources` ()
- V4D_Mod:: `CreateVulkanResources2` ()
- Create Descriptor Sets
- V4D_Mod:: `CreateVulkanPipelines` ()
- EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
- V4D_Mod:: `CreateVulkanCommandBuffers` ()
- EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
- Start Server
- Start Client
- Start threads and game loops
#### Quit
- Stop Server
- EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
- V4D_Mod:: `DestroyVulkanCommandBuffers` ()
- EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
- V4D_Mod:: `DestroyVulkanPipelines` ()
- Destroy Descriptor Sets
- V4D_Mod:: `DestroyVulkanResources2` ()
- V4D_Mod:: `DestroyVulkanResources` ()
- V4D_Mod:: `FreeVulkanBuffers` ()
- Destroy Command Pools
- Destroy SwapChain
- V4D_Mod:: `DestroyVulkanSyncObjects` ()
- Destroy Allocator
- Destroy Rendering Device
- V4D_Mod:: `InputCallbackName` ()
- Remove Input Callbacks
- V4D_Mod:: `UnloadScene` ()
- Destroy Scene
- Destroy Renderer
- Destroy Window
- Destroy Vulkan Instance
- V4D_Mod:: `ModuleUnload` ()
#### Reload Renderer
- EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
- V4D_Mod:: `DestroyVulkanCommandBuffers` ()
- EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
- V4D_Mod:: `DestroyVulkanPipelines` ()
- Destroy Descriptor Sets
- V4D_Mod:: `DestroyVulkanResources2` ()
- V4D_Mod:: `DestroyVulkanResources` ()
- V4D_Mod:: `FreeVulkanBuffers` ()
- Destroy Command Pools
- Destroy SwapChain
- V4D_Mod:: `DestroyVulkanSyncObjects` ()
- Destroy Allocator
- Destroy Rendering Device
- EVENT `v4d::graphics::renderer::event::Reload` (Renderer*)
- V4D_Mod:: `ReadShaders` ()
- V4D_Mod:: `ScorePhysicalDeviceSelection` ()
- V4D_Mod:: `InitVulkanDeviceFeatures` ()
- V4D_Mod:: `InitRenderingDevice` ()
- Create Rendering Device
- Create Allocator
- V4D_Mod:: `ConfigureRenderer` ()
- V4D_Mod:: `CreateVulkanSyncObjects` ()
- Create SwapChain
- Create Command Pools
- V4D_Mod:: `AllocateVulkanBuffers` ()
- V4D_Mod:: `CreateVulkanResources` ()
- V4D_Mod:: `CreateVulkanResources2` ()
- Create Descriptor Sets
- V4D_Mod:: `CreateVulkanPipelines` ()
- EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
- V4D_Mod:: `CreateVulkanCommandBuffers` ()
- EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
#### Slow Game Loop
- V4D_Mod:: `SlowLoopUpdate` ()
#### Game Loop
- V4D_Mod:: `GameLoopUpdate` ()
#### Primary Rendering Loop
- V4D_Mod:: `RenderUpdate` ()  
    **Main Rendering Module:**
    - Aquire next swapchain image
    - Wait for fence signaled by previous frame's Push commands
    - Set camera options and projection matrix
    - V4D_Mod:: `RenderFrame_BeforeUpdate` ()  
        - Modules may create new entities here  
        - Modules may set the camera's view matrix here  
    - Generate entities via their generator function
    - Wait for fence signaled by previous frame's Pull commands
    - V4D_Mod:: `OnRendererRayCastOut` ()  
    - V4D_Mod:: `OnRendererRayCastHit` ()  
    *TODO catch collision events here*
    - Cleanup destroyed entities
    - V4D_Mod:: `RenderFrame_Update` ()
    - Loop through scene objects
        - Prepare BLAS build if not already built
        - Apply physics (velocity, collisions, constraints, gravity, forces)
        - Prepare collision buffer for processing on the GPU via Ray-Tracing
        - Prepare TLAS instance
        - Cache important light sources in a buffer
    - Record Push Commands
        - Push buffers to GPU
        - V4D_Mod:: `RenderFrame_Push` ()
    - Submit Push commands
    - Record Build Commands
        - Build Acceleration Structures
        - V4D_Mod:: `RenderFrame_Build` ()
    - Submit Build commands
    - Record Graphics Commands
        - Run ray-traced rendering
        - V4D_Mod:: `RenderFrame_Graphics` ()
        - Run `sg_transparent` shaders (rasterization)
        - Run `sg_wireframe` shaders (rasterization)
    - Submit Graphics commands
    - Record Physics Commands
        - Run ray-traced collision detection
        - V4D_Mod:: `RenderFrame_Physics` ()
    - Submit Physics commands
    - Record Pull Commands
        - Pull collision buffer
        - Pull raycast buffer
        - V4D_Mod:: `RenderFrame_Pull` ()
    - Submit Pull commands
    - Wait for fence signaled by previous frame's Post commands
    - Record Post Commands
        - V4D_Mod:: `RenderFrame_Post` ()
        - Run `sg_present` shaders (rasterization)
        - Copy render images to history
        - Generate thumbnail
    - Submit Post commands
    - Present image to screen
#### Secondary Rendering Loop
- V4D_Mod:: `DrawUi` ()
- V4D_Mod:: `SecondaryRenderUpdate` ()
    - Read histogram that was computed in previous frame
    - V4D_Mod:: `BeginSecondaryFrameUpdate` ()
    - Begin single time command buffer
        - Compute histogram from previous frame
        - V4D_Mod:: `SecondaryFrameCompute` ()
        - V4D_Mod:: `SecondaryRenderUpdate2` () sorted by `render`
    - Submit command buffer
#### Input Loop
- V4D_Mod:: `InputUpdate` ()


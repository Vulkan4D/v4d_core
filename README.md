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
- `common/*` common headers (mostly std stuff)
- `helpers/*` Contains header-only `.hpp` files with helper methods
- `modules/*` Contains module classes for modding
- `utilities/*` Contains subdirectories for utility categories with all their utilities
- `Core.cpp` Core Source File compiled only in the core library
- `Core.h` Core header file
- `v4d.h` Main Header file to be included in anything that is part of V4D
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
 - main()
    - Instantiate V4D_Renderer
    - V4D_Renderer:: `Init` ()
    - V4D_Game:: `Init` ()
    - V4D_Physics:: `Init` ()
    - V4D_Input:: `Init` ()
    - V4D_Input:: `AddCallbacks` ()
    - V4D_Physics:: `LoadScene` ()
    - V4D_Game:: `LoadScene` ()
    - Renderer:: `InitRenderer` ()
        - V4D_Renderer:: `InitLayouts` ()
        - V4D_Renderer:: `ConfigureShaders` ()
    - Renderer:: `ReadShaders` ()
        - V4D_Renderer:: `ReadShaders` ()
    - Renderer:: `LoadRenderer` ()
        - Renderer:: `CreateDevices` ()
            - V4D_Renderer:: `InitDeviceFeatures` ()
        - V4D_Renderer:: `ConfigureRenderer` ()
        - Renderer:: `CreateSyncObjects` ()
        - Renderer:: `CreateSwapChain` ()
        - Renderer:: `LoadGraphicsToDevice` ()
            - Renderer:: `CreateCommandPools` ()
            - V4D_Renderer:: `AllocateBuffers` ()
            - V4D_Renderer:: `CreateResources` () PRIMARY
                - V4D_Game:: `RendererCreateResources` ()
            - Renderer:: `CreateDescriptorSets` ()
                - Renderer:: `UpdateDescriptorSets` ()
            - V4D_Renderer:: `CreatePipelines` ()
            - EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
            - V4D_Renderer:: `CreateCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
    - Start threads and game loops
#### Quit
 - Stop game loops and Join threads
 - V4D_Input:: `RemoveCallbacks` ()
 - Renderer:: `UnloadRenderer` ()
    - EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
    - Renderer:: `UnloadGraphicsFromDevice` ()
        - V4D_Renderer:: `DestroyCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
        - V4D_Renderer:: `DestroyPipelines` ()
        - Renderer:: `DestroyDescriptorSets` ()
        - V4D_Renderer:: `DestroyResources` () PRIMARY
            - V4D_Game:: `RendererDestroyResources` ()
        - V4D_Renderer:: `FreeBuffers` ()
        - Renderer:: `DestroyCommandPools` ()
    - Renderer:: `DestroySwapChain` ()
    - Renderer:: `DestroySyncObjects` ()
    - Renderer:: `DestroyDevices` ()
 - V4D_Game:: `UnloadScene` ()
 - V4D_Physics:: `UnloadScene` ()
#### Reload
 - Renderer:: `ReloadRenderer` ()
    - EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
    - Renderer:: `UnloadGraphicsFromDevice` ()
        - V4D_Renderer:: `DestroyCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
        - V4D_Renderer:: `DestroyPipelines` ()
        - Renderer:: `DestroyDescriptorSets` ()
        - V4D_Renderer:: `DestroyResources` () PRIMARY
            - V4D_Game:: `RendererDestroyResources` ()
        - V4D_Renderer:: `FreeBuffers` ()
        - Renderer:: `DestroyCommandPools` ()
    - Renderer:: `DestroySwapChain` ()
    - Renderer:: `DestroySyncObjects` ()
    - Renderer:: `DestroyDevices` ()
    - EVENT `v4d::graphics::renderer::event::Reload` (Renderer*)
    - V4D_Renderer:: `ReadShaders` ()
    - Renderer:: `CreateDevices` ()
        - V4D_Renderer:: `InitDeviceFeatures` ()
    - V4D_Renderer:: `ConfigureRenderer` ()
    - Renderer:: `CreateSyncObjects` ()
    - Renderer:: `CreateSwapChain` ()
    - Renderer:: `LoadGraphicsToDevice` ()
        - Renderer:: `CreateCommandPools` ()
        - V4D_Renderer:: `AllocateBuffers` ()
        - V4D_Renderer:: `CreateResources` () PRIMARY
            - V4D_Game:: `RendererCreateResources` ()
        - Renderer:: `CreateDescriptorSets` ()
            - Renderer:: `UpdateDescriptorSets` ()
        - V4D_Renderer:: `CreatePipelines` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
        - V4D_Renderer:: `CreateCommandBuffers` ()
    - EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
#### Slow Game Loop
    - V4D_Game:: `SlowUpdate` ()
    - V4D_Physics:: `SlowStepSimulation` ()
#### Game Loop
    - V4D_Game:: `Update` ()
    - V4D_Physics:: `StepSimulation` ()
#### Frame Update 2 (secondary rendering)
    - V4D_Renderer:: `RunUi` () PRIMARY
        - V4D_Game:: `RendererRunUi` ()
        - V4D_Game:: `RendererRunUiDebug` ()
    - V4D_Physics:: `RunUi` ()
    - V4D_Renderer:: `Update2` () PRIMARY
        - V4D_Game:: `RendererFrameUpdate2` ()
        - V4D_Game:: `RendererFrameCompute` ()
        - V4D_Renderer:: `Render2` ()
#### Frame Update (main rendering)
    - Renderer:: `Update` ()
        - V4D_Renderer:: `Update` () PRIMARY
            - V4D_Game:: `RendererFrameUpdate` ()
            - V4D_Renderer:: `Render` ()
#### Input Loop
    - V4D_Input:: `Update` ()


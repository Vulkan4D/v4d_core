# v4d_core
Source files for **Vulkan4D Core Library** (Core Utilities and Helper functions)

These source files compile into `v4d.dll`

It must be linked with when building a game using the Vulkan4D Engine.


# Vulkan4D
Vulkan4D is a revolutionary game engine built from the ground up for Space Games/Simulations and with Vulkan as the sole rendering API, so that we can take full advantage of the new technology. 


### Project Structure
- `Core` Compiled into `v4d.dll` and linked into the Project
- `Helpers` Simple-but-useful header-only source files, compiled into anything that is part of V4D
- `Modules` Game functionalities (and plugins/mods) compiled into individual .dll files that are loaded at runtime
- `Libraries` Other libraries used in the project
- `Resources` Icons, Textures, Music, ...
- `Project` App that puts it all together to run the game
- `Tools` Useful tools to help programmers (build scripts, shader compiler, ...)


## V4D Core Utilities

Core Utilities are a solid part of Vulkan4D's Core to provide developers with interfaces to the Hardware. 

They are split into 5 categories (`Audio`, `Graphics`, `IO`, `Networking`, `Processing`)


## File Structure
The core consists of the following structure :
- `helpers/*` Contains header-only `.hpp` files with helper methods
- `utilities/*` Contains subdirectories for utility categories with all their utilities, each consisting of up to three files with the same [Utility] name and different extensions (`.cpp`, `.h`, `.cxx`) and other subdirectories such as `assets/`
- `v4d.h` Main Header file to be included in anything that is part of V4D
- `Core.h` Core header file
- `Core.cpp` Core Source File compiled only in the core library
- `tests.cxx` Unit Tests
- `README.md` this documentation
- `common/*` common headers (mostly std stuff)
- `*.hh` Header files included in v4d.h


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


## Renderer

### Method execution order
#### Load
 - main()
    - Instantiate V4DRenderer
    - Renderer:: `InitRenderer` ()
        - V4DRenderer:: `Init` ()
        - V4DRenderer:: `InitLayouts` ()
        - V4DRenderer:: `ConfigureShaders` ()
    - V4DRenderer:: `ReadShaders` ()
    - V4DRenderer:: `LoadScene` ()
    - Renderer:: `LoadRenderer` ()
        - Renderer:: `CreateDevices` ()
        - V4DRenderer:: `Info` ()
        - Renderer:: `CreateSyncObjects` ()
        - Renderer:: `CreateSwapChain` ()
        - Renderer:: `LoadGraphicsToDevice` ()
            - Renderer:: `CreateCommandPools` ()
            - V4DRenderer:: `CreateResources` ()
            - V4DRenderer:: `AllocateBuffers` ()
            - Renderer:: `CreateDescriptorSets` ()
                - Renderer:: `UpdateDescriptorSets` ()
            - V4DRenderer:: `CreatePipelines` ()
            - EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
            - Renderer:: `CreateCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
#### Unload
 - Renderer:: `UnloadRenderer` ()
    - EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
    - Renderer:: `UnloadGraphicsFromDevice` ()
        - Renderer:: `DestroyCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
        - V4DRenderer:: `DestroyPipelines` ()
        - Renderer:: `DestroyDescriptorSets` ()
        - V4DRenderer:: `FreeBuffers` ()
        - V4DRenderer:: `DestroyResources` ()
        - Renderer:: `DestroyCommandPools` ()
    - Renderer:: `DestroySwapChain` ()
    - Renderer:: `DestroySyncObjects` ()
    - Renderer:: `DestroyDevices` ()
#### Reload
 - Renderer:: `ReloadRenderer` ()
    - EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
    - Renderer:: `UnloadGraphicsFromDevice` ()
        - Renderer:: `DestroyCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
        - V4DRenderer:: `DestroyPipelines` ()
        - Renderer:: `DestroyDescriptorSets` ()
        - V4DRenderer:: `FreeBuffers` ()
        - V4DRenderer:: `DestroyResources` ()
        - Renderer:: `DestroyCommandPools` ()
    - Renderer:: `DestroySwapChain` ()
    - Renderer:: `DestroySyncObjects` ()
    - Renderer:: `DestroyDevices` ()
    - EVENT `v4d::graphics::renderer::event::Reload` (Renderer*)
    - V4DRenderer:: `ReadShaders` ()
    - Renderer:: `CreateDevices` ()
    - V4DRenderer:: `Info` ()
    - Renderer:: `CreateSyncObjects` ()
    - Renderer:: `CreateSwapChain` ()
    - Renderer:: `LoadGraphicsToDevice` ()
        - Renderer:: `CreateCommandPools` ()
        - V4DRenderer:: `CreateResources` ()
        - V4DRenderer:: `AllocateBuffers` ()
        - Renderer:: `CreateDescriptorSets` ()
            - Renderer:: `UpdateDescriptorSets` ()
        - V4DRenderer:: `CreatePipelines` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
        - Renderer:: `CreateCommandBuffers` ()
    - EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
#### Screen Resize
 - Renderer:: `RecreateSwapChains` ()
    - EVENT `v4d::graphics::renderer::event::Unload` (Renderer*)
    - Renderer:: `UnloadGraphicsFromDevice` ()
        - Renderer:: `DestroyCommandBuffers` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesDestroy` (Renderer*)
        - V4DRenderer:: `DestroyPipelines` ()
        - Renderer:: `DestroyDescriptorSets` ()
        - V4DRenderer:: `FreeBuffers` ()
        - V4DRenderer:: `DestroyResources` ()
        - Renderer:: `DestroyCommandPools` ()
    - EVENT `v4d::graphics::renderer::event::Resize` (Renderer*)
    - Renderer:: `CreateSwapChain` ()
    - Renderer:: `LoadGraphicsToDevice` ()
        - Renderer:: `CreateCommandPools` ()
        - V4DRenderer:: `CreateResources` ()
        - V4DRenderer:: `AllocateBuffers` ()
        - Renderer:: `CreateDescriptorSets` ()
            - Renderer:: `UpdateDescriptorSets` ()
        - V4DRenderer:: `CreatePipelines` ()
        - EVENT `v4d::graphics::renderer::event::PipelinesCreate` (Renderer*)
        - Renderer:: `CreateCommandBuffers` ()
    - EVENT `v4d::graphics::renderer::event::Load` (Renderer*)
#### Frame Update
 - main()
    - Renderer:: `Render` ()
        - [ conditional call of `RecreateSwapChains`() and return ]
        - [ conditional call of `ReloadRenderer`() and return ]
        - V4DRenderer:: `FrameUpdate`()
        - V4DRenderer:: `BeforeGraphics`()
        - V4DRenderer:: `RunDynamicGraphics`()
        - *( submit dynamic commands, wait for semaphores, then submit pre-recorded commands )*
        - *( present )*


# v4d_core
Source files for **Vulkan4D Core Library** (Core Modules and Helper functions)

These source files compile into `v4d.dll`

It must be linked with when building a game using the Vulkan4D Engine.


# Vulkan4D
Vulkan4D is a revolutionary game engine built from the ground up for Space Games/Simulations and with Vulkan as the sole rendering API, so that we can take full advantage of the new technology. 

### Project Structure
- `Core Modules and Helpers` Compiled into `v4d.dll` and linked into the Launcher
- `Systems` Game functionalities (and mods) compiled into individual .dll files that are loaded at runtime
- `Libraries` Other libraries used in the project
- `Resources` Icons, Textures, Music, ...
- `Launcher` App that puts it all together to run the game


## V4D Core Modules

Core Modules are a solid part of Vulkan4D's Core to provide developers with interfaces to the Hardware. 

They are split into six categories (`Audio`, `FileSystem`, `Graphics`, `Input`, `Networking`, `Processing`)


## File Structure
The core consists of the following structure :
- `helpers/` Contains header-only (.hpp) files with helper methods
- `modules/` Contains subdirectories for module categories with all their modules, each consisting of up to three files with the same [Module] name and different extensions (.cpp, .h, .cxx) and other subdirectories such as `shaders/`
- `v4d.h` Main Header file to be included when linking against this library
- `v4d.cpp` Main Source File compiled only in the core library
- `tests.cxx` Unit Tests
- `README.md` this documentation


## Coding Standards

### Formatting
- {} brackets start on the SAME LINE, preceded by a space
- Space after keywords (if, for, while,...)
- Space before and After Stream operators (<< >>)
- Indentations are 4 spaces

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
- iterators are i,j,k and x,y,z,w

### File Extensions
- .cpp C++ Source Files compiled via g++, must have a matching .h (except main file)
- .hpp Header-only source files (no matching .cpp)
- .h C++ Header files included from a matching .cpp source file
- .cxx C++ Tests included in debug mode only


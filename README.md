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
- `{}` brackets start on the SAME LINE, preceded by a space
    ```c++
    int main(int argc, char** args) {
        std::cout << "Hello V4D !" << std::endl;
    }
    ```
- methods and conditions on multiple lines are all or nothing (all in one line, or one line per argument)
    ```c++
    void someLongMethodWithLotsOfArguments(
        const int& a,
        const int& b,
        const std::string& str
    ) {
        if (
            a == b
            &&
            (
                someCondition
                ||
                someOtherCondition
            )
        ) {
            // ...
        }
    }
    ```
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
- iterators are `i`,`j`,`k` and `x`,`y`,`z`,`w`

### File Extensions
- .cpp C++ Source Files compiled via g++, must have a matching .h (except main file)
- .hpp Header-only source files (no matching .cpp)
- .h C++ Header files included from a matching .cpp source file
- .cxx C++ Reserved for Unit Tests


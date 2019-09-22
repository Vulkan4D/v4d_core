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
- `utilities/*` Contains subdirectories for utility categories with all their utilities, each consisting of up to three files with the same [Utility] name and different extensions (`.cpp`, `.h`, `.cxx`) and other subdirectories such as `shaders/` or `res/`
- `v4d.h` Main Header file to be included in anything that is part of V4D
- `Core.h` Core header file
- `Core.cpp` Core Source File compiled only in the core library
- `tests.cxx` Unit Tests
- `README.md` this documentation
- `common/*` precompiled headers
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
- iterators are `i`,`j`,`k` and `x`,`y`,`z`,`w`
- Unsafe method/member names are surrounded by underscores : \_UnsafeMethod\_()
- Public method/member names intended for internal use start with two underscores : __SomeMethod()

### File Extensions
- `.cpp` C++ Source Files compiled via g++, must have a matching .h (except main file)
- `.hpp` C++ Header-only source files (no matching .cpp)
- `.h` C++ Header files included from a matching .cpp source file
- `.cxx` C++ source files Reserved for Unit Tests
- `.hh` C++ header-only files without any associated code


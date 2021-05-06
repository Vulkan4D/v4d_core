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
- `V4D_Mod.h/cpp` Modding interface for V4D
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
        - Reference definitions : `int& a = b;`
        - Pointer type arguments in methods : `void func(int* arg) {`
        - Argument by reference in methods : `void func(int& arg) {`
    - Dereferencing and getting the address have the symbol sticking on the variable name
        - Assigning the referenced value of a pointer variable : `int val = *prt;`
        - Assigning a pointer to the address of a variable : `int* var2 = &var1;`
        - Assigning a value to a dereferenced pointer : `*ptr = 5;`

### Rules and modern conventions
- as much as possible, use C++20 features
- struct/class initialized using braces `{ }` and generic types using `=`
- use `static_cast<>` instead of c-style casting for struct/class types
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
- static const variables and macros are ALL_CAPS
- method arguments are camelCase with the same name as an underlying class member that it constructs
- file names unrelated to a class are all_lowercase
- Unsafe/temporary method/member names are surrounded by underscores : \_UnsafeMethod\_()
- Public method/member names (or macros) intended only for internal use should start with `__V4D_...`

### File Extensions
- `.cpp` C++ Source Files with a matching .h (except main.cpp)
- `.hpp` C++ Header-only source files (no matching .cpp)
- `.h` C++ Header files included from a matching .cpp source file (except v4d.h)
- `.cxx` C++ source files Reserved for Unit Tests *(Subject to change)*
- `.hh` C++ header-only files without any associated code (config files, enum, basic structs,...)

### Critical method names for correct memory management
- `Create*` must be followed by `Destroy*`
- `New*` must be followed by `Delete*`
- `Allocate*` must be followed by `Free*`
- `Begin*` must be followed by `End*`
- `Start*` must be followed by `Stop*` or `Restart*`
- `Load*` must be followed by `Unload*` or `Reload*` (`Load` should not be called more than once unless after an `Unload`)
- `Init*` must only be called once per instance, and manages its own memory
- `Configure*`, `Generate*`, `Make*`, `Build*`, `Read*`, `Write*` may be called any number of times, and manages its own memory



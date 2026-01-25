# Contributing Guide

Thank you very much for your interest in contributing to the Controllers library! Here's a small guide that will help you with getting started on your development endevors.

If you have any doubts, do not hesitate on cotacting us by opening an issue on this repository, or via email at [trasgo@infor.uva.es](mailto://trasgo@infor.uva.es).

## Coding style

Formatting is done via [clang-format](https://clang.llvm.org/docs/ClangFormat.html), with a wrapper script (`format.sh`) to be able to indent preprocessor directives in the middle of the code. See [the formatiing file](./.clang-format) for details on the formatting style used.

If the formatter is misbehaving for a specific file or section of code, feel free to turn it off using `clang-format <on/off>`, but address the reason for turning it off in a comment.

Controllers uses the following naming convention:
- Variables: `snake_case` - e.g. `variable_name`
- C pointer: `snake_case` with prefix `p_` - e.g. `p_pointer_name`
- C pointer of pointer: `snake_case` with prefix `pp_` - e.g. `pp_pointer_of_pointer_name`
- Functions: prefix `Ctrl_` followed by module name (e.g., Cuda, OpenCLGpu) and function name both `PascalCase` - e.g. `Ctrl_ModuleName_FunctionName`
- Enum and struct types: same as functions - e.g. `Ctrl_ModuleName_TypeName`
- Compile-time constants: `_CTRL_` prefix followed by constant name in `UPPER_SNAKE_CASE` and underscore suffix `_` (in the case of header constants, add `H_` suffix) - e.g. `_CTRL_CONSTANT_NAME_[H_]`

## Adding a new Controller

A _Controller_ is an entity that manages a certain computing accelerator kind, using a specific runtime library (backend). If you want to add support for a new computing device kind, or for a specific heterogeneous runtime technology, you will need to add a new Controller to the library.
Please refer to the implementation of the already existing Controllers as a general guide for developing a new one.

To begin with, create a new folder with the new Controller name at paths:
- `Ctrl/inc/Architectures` - Headers for the controller type and tile extension for it.
- `Ctrl/src/Architectures` - Source code for the Controller.
- `Ctrl/inc/Kernel/Architectures` - Headers for kernels prototyping, wrapper and launch.

Inside `Ctrl/inc/Architectures` you should write headers defining the structure of your controller. These headers files should also define the structure with additional information about the tile.

Inside `Ctrl/src/Architectures` you should write the logic and source code for the new Controller type operations.

Inside `Ctrl/inc/Kernel/Architectures` your headers should define the macros used to generate the kernels and handle their execution.

Some changes to the Core parts of Controllers (`Ctrl/src/Core`, `Ctrl/inc/Core` and `Ctrl/inc/Kernel`) may be required. You can look for the `_CTRL_ARCH_<arch>_` symbol usages to find where these changes may be required.

**Important:** All headers should be once-only headers - they should be encapsulled into an `#ifndef`region driven by header constant. More information [here](https://gcc.gnu.org/onlinedocs/cpp/Once-Only-Headers.html).
```c
/* File foo.  */
#ifndef _CTRL_FILE_FOO_H_
#define _CTRL_FILE_FOO_H_

the entire file

#endif // _CTRL_FILE_FOO_H_
```

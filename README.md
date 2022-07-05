# [impeller-cmake](https://github.com/bdero/impeller-cmake) Example Project

## Supported build environments

This CMake project compiles parts of Flutter Engine, including Impeller's renderer layer and FML. The Flutter Engine codebase _only_ supports building against **Clang**.

### Windows

This repository is tested against the `x64 Native Tools Command Prompt for VS 2022`.

### macOS

This repository is tested against **Homebrew Clang 13.0.0**, which ships with the homebrew `llvm` package.

## Fetching dependencies

1. Clone the repository and fetch submodules:

    ```sh
    git clone git@github.com:bdero/impeller-cmake-example.git --recursive
    ```
1. Fetch non-submodule dependencies:

    ```sh
    cd impeller-cmake-example
    ./deps.sh
    ```

## Developing with vscode

This repository contains configuration to make working with vscode easy out of the box (`.vscode/settings.json` and `CMakePresets.json`).

Note that ubiquitous tools like `cmake` and `clangd` do all of the heavy lifting here, and getting a good code completion/navigation experience is possible for any editor with `clangd` support (Emacs, Sublime, etc).

1. Get clang:
   * **Windows:** Install Visual Studio 2022 and open the `x64 Native Tools Command Prompt for VS 2022`
   * **macOS:** Download LLVM (homebrew: `brew install llvm`)
1. Install recommended vscode extensions:
   * [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) (build using CMake presets)
   * [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (accurate code completion and navigation)
1. Open the root directory of this repository in vscode:

    ```sh
    code impeller-cmake-example
    ```
1. Configure CMake (bottom bar).
    1. Select the active configure preset:
       * **Windows:** `Ninja Debug VS2022`
       * **macOS:** `Ninja Debug`
    1. Set the active build preset to `Example debug`.
    1. Set the default target to `example`.
1. Build/debug/run and enjoy!

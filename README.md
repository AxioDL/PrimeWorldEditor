# Prime World Editor
Prime World Editor is a custom editor suite for Retro Studios' GameCube and Wii games, including the
Metroid Prime series and Donkey Kong Country Returns.

# Clone Submodules First!

Builders on all platforms should ensure submodules are up to date with the current PrimeWorldEditor
by running `git submodule update --init --recursive`.

# Building on Windows

## Requirements

**64-bit Note:** Except for Visual Studio and the Qt installer, ensure 64-bit (x86-64, x64) packages are installed.

* [Visual Studio 2017](https://visualstudio.microsoft.com/vs/) (Desktop development with C++)
* [Qt 5.10+](https://download.qt.io/official_releases/qt/) (MSVC 2017 64-bit specification)
* [LLVM 6.0.1 x86-64](http://releases.llvm.org/6.0.1/LLVM-6.0.1-win64.exe) installation; currently must be installed to `C:\Program Files\LLVM\`
* [Python 3 x86-64](https://www.python.org/downloads/windows/)

## Build using Qt Creator

**Compiler Note:** Before starting, it is wise to select the 64-bit compiler by opening *Tools* > *Kits* > *Kits* tab.
The C and C++ compilers should be set to *Microsoft Visual C++ Compiler 15.0 (amd64)*.

**CMake Note:** At the time of writing, Qt Creator will search for a user-installed CMake but this is not necessary. 
The CMake that comes with Visual Studio may be manually selected by opening *Tools* > *Kits* > *CMake* tab, then adding
a manual CMake with the path:
`C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`.

**Debugger Note:** To use Qt Creator for debugging, *Debugging Tools for Windows* must be installed from the
[Windows SDK package](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk). Once installed,
the Qt Creator kit should automatically detect CDB. Make sure the x64 version is selected.

1. *File* > *Open File or Project*
2. Select *CMakeLists.txt* at root of cloned PrimeWorldEditor
3. Select desired build configurations within the Desktop Qt Kit. It is generally fine to just check *Debug* and *Release*.
4. Click *Configure Project*
5. Wait for dependencies to build and CMake project to generate (this may take a while).
6. Edit/Build/Debug/Run

## Build using Visual Studio

**Qt Note:** It may be necessary to edit both CMAKE_PREFIX_PATH entries in the *CMakeSettings.json* file.
They should be set to `C:/Qt/<QT VERSION>/msvc2017_64/lib/cmake/Qt5`.

1. *File* > *Open* > *CMake*
2. Select *CMakeLists.txt* at root of cloned PrimeWorldEditor
3. Wait for dependencies to build and CMake project to generate (this may take a while).
4. Edit/Build/Debug/Run

## Build using Command Line

1. Launch *x64 Native Tools Command Prompt for VS 2017*
2. `cd <PATH TO PrimeWorldEditor ROOT>`
3. `mkdir build`
4. `cd build`
5. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/<QT VERSION>/msvc2017_64/lib/cmake/Qt5 ..`
6. `ninja`
7. *PrimeWorldEditor.exe* is found in the `build/bin` directory.

# Building on macOS

## Requirements

* [Xcode 10.2+](https://developer.apple.com/xcode/)
* [Qt 5.10+](https://download.qt.io/official_releases/qt/) (macOS specification)
* *cmake*, *ninja* and *python* installed using [Homebrew](https://brew.sh/)

## Build using Xcode

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Xcode -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/Qt/<QT VERSION>/clang_64/lib/cmake/Qt5 ..`
5. Open *PrimeWorldEditor.xcodeproj*
6. Edit/Build/Debug/Run

## Build using Command Line

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/Qt/<QT VERSION>/clang_64/lib/cmake/Qt5 ..`
5. `ninja`
6. *PrimeWorldEditor.app* is found in the `build/bin` directory.

# Building on Linux

## Requirements

* A working compiler toolchain (GCC or Clang)
* *cmake*, *ninja*, *python3*, *qt5* (dev), *clang* (dev) packages

## Build using Command Line

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..`
5. `ninja`
6. *PrimeWorldEditor* is found in the `build/bin` directory.

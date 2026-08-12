# Prime World Editor
Prime World Editor is a custom editor suite for Retro Studios' GameCube and Wii games, including the
Metroid Prime series and Donkey Kong Country Returns.

# Clone Submodules First!

Builders on all platforms should ensure submodules are up to date with the current PrimeWorldEditor
by running `git submodule update --init --recursive`.

# Building on Windows

## Requirements

**64-bit Note:** Except for Visual Studio and the Qt installer, ensure 64-bit (x86-64, x64) packages are installed.

* [Visual Studio 2026](https://visualstudio.microsoft.com/vs/) (Desktop development with C++)
* [Qt 6.5+](https://www.qt.io/development/download-qt-installer-oss) (MSVC 2022 64-bit specification)
  * Yes, MSVC 2022, since a Qt version with MSVC 2026 libs isn't provided yet as of writing. The ABI is still stable nonetheless.

## Build using Qt Creator

**Compiler Note:** Before starting, it is wise to select the 64-bit compiler by opening *Tools* > *Kits* > *Kits* tab.
The C and C++ compilers should be set to *Visual Studio Community 2026 (amd64)*.

**CMake Note:** At the time of writing, Qt Creator will search for a user-installed CMake but this is not necessary. 
The CMake that comes with Visual Studio may be manually selected by opening *Tools* > *Kits* > *CMake* tab, then adding
a manual CMake with the path:
`C:\Program Files (x86)\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`.

**Debugger Note:** To use Qt Creator for debugging, *Debugging Tools for Windows* must be installed from the
[Windows SDK package](https://learn.microsoft.com/en-ca/windows/apps/windows-sdk/). Once installed,
the Qt Creator kit should automatically detect CDB. Make sure the x64 version is selected.

1. *File* > *Open File or Project*
2. Select *CMakeLists.txt* at root of cloned PrimeWorldEditor
3. Select desired build configurations within the Desktop Qt Kit. It is generally fine to just check *Debug* and *Release*.
4. Click *Configure Project*
5. Wait for dependencies to build and CMake project to generate (this may take a while).
6. Edit/Build/Debug/Run

## Build using Visual Studio

**Qt Note:** It may be necessary to edit both CMAKE_PREFIX_PATH entries in the *CMakeSettings.json* file.
They should be set to `C:/Qt/<QT VERSION>/msvc2022_64/lib/cmake/Qt6`.

1. *File* > *Open* > *CMake*
2. Select *CMakeLists.txt* at root of cloned PrimeWorldEditor
3. Wait for dependencies to build and CMake project to generate (this may take a while).
4. Edit/Build/Debug/Run

## Build using Command Line

1. Launch *x64 Native Tools Command Prompt for VS 2026*
2. `cd <PATH TO PrimeWorldEditor ROOT>`
3. `mkdir build`
4. `cd build`
5. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=<PATH TO QT ROOT>/<QT VERSION>/msvc2022_64/lib/cmake/Qt6 ..`
6. `ninja`
7. *PrimeWorldEditor.exe* is found in the `build/bin` directory.

Note that nothing is wrong if the CMake step is taking longer than usual. It's likely just downloading dependencies necessary for building PWE.

# Building on macOS

## Requirements

* [Xcode 10.2+](https://developer.apple.com/xcode/)
* [Qt 6.5+](https://www.qt.io/development/download-qt-installer-oss) (macOS specification)
* *cmake* and *ninja* installed using [Homebrew](https://brew.sh/)

## Build using Xcode

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Xcode -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=<PATH TO QT ROOT>/<QT VERSION>/clang_64/lib/cmake/Qt6 ..`
5. Open *PrimeWorldEditor.xcodeproj*
6. Edit/Build/Debug/Run

## Build using Command Line

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF -DCMAKE_PREFIX_PATH=<PATH TO QT ROOT>/<QT VERSION>/clang_64/lib/cmake/Qt6 ..`
5. `ninja`
6. *PrimeWorldEditor.app* is found in the `build/bin` directory.

# Building on Linux

## Requirements

* A working compiler toolchain (GCC or Clang)
* *cmake*, *ninja*, *qt6* (dev)

## Build using Command Line

1. `cd <PATH TO PrimeWorldEditor ROOT>`
2. `mkdir build`
3. `cd build`
4. `cmake -G Ninja -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release ..`
5. `ninja`
6. *PrimeWorldEditor* is found in the `build/bin` directory.

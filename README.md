# Sparkle

Sparkle is a C++20 static library that bundles a resource pipeline and an OpenGL rendering layer. The project is driven by CMake, uses Ninja as its default generator, and relies on vcpkg to resolve third-party dependencies (GLEW, OpenGL, stb, and GoogleTest for optional unit tests).

## 1. Prepare Your Environment

Follow the numbered subsections below to install each prerequisite before building Sparkle.

### 1.1 Install a C++ Toolchain (MSVC)

1. Download and install [Visual Studio 2022 Build Tools](https://visualstudio.microsoft.com/downloads/).
2. During setup, enable the **Desktop development with C++** workload to obtain the MSVC compiler, Windows SDK, and Ninja support.
3. Open a new `x64 Native Tools Command Prompt` or PowerShell session and verify the compiler is available:
   ```powershell
   cl
   ```
   The command should print the compiler version banner instead of an error.

### 1.2 Install vcpkg

Sparkle uses vcpkg in manifest mode to fetch GLEW, OpenGL, stb, and GoogleTest.

1. Clone the vcpkg repository:
   ```powershell
   git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
   ```
2. Bootstrap vcpkg:
   ```powershell
   & C:\dev\vcpkg\bootstrap-vcpkg.bat
   ```
3. Make the installation discoverable by CMake (per-session example):
   ```powershell
   $env:VCPKG_ROOT = 'C:\dev\vcpkg'
   ```
   Optionally, add `VCPKG_ROOT` to your global environment variables so it persists across terminals.

### 1.3 Install CMake ≥ 3.16

1. Download the Windows x64 installer from the [CMake downloads page](https://cmake.org/download/).
2. Run the installer and choose **Add CMake to the system PATH** when prompted.
3. Confirm the version:
   ```powershell
   cmake --version
   ```

### 1.4 (Optional) Install Doxygen and Graphviz

1. Use your preferred package manager to install Doxygen:
   ```powershell
   choco install doxygen.portable
   ```
   Alternatively, download the Windows installer from [doxygen.nl](https://www.doxygen.nl/download.html) and follow the setup steps.
2. (Optional) Install Graphviz to enable diagram generation:
   ```powershell
   choco install graphviz
   ```
3. Confirm both tools are available:
   ```powershell
   doxygen --version
   dot -V
   ```

### 1.5 Install LLVM (Clang + clang-tidy)

Sparkle's static analysis pipeline relies on clang-tidy. Install LLVM to get both the Clang compiler and clang-tidy command-line tool:

1. Install the LLVM toolchain:
   ```powershell
   choco install llvm
   ```
   If you prefer manual installation, download the Windows package from the [LLVM releases page](https://releases.llvm.org/) and follow the installer steps.
2. Verify the tools are on your `PATH`:
   ```powershell
   clang --version
   clang-tidy --version
   ```
   Both commands should print version banners. The `clang-tidy` preset described below requires these binaries.

## 2. Build Sparkle

1. Clone the repository (skip if you already have a working copy):
   ```powershell
   git clone https://github.com/YourOrganization/Sparkle.git
   cd Sparkle
   ```
2. Configure the project with one of the supplied presets:
   ```powershell
   cmake --preset release      # or debug
   ```
   The presets automatically point CMake to `VCPKG_ROOT`, enable C++20, and cap object-path lengths for Windows builds.
3. Build the library and tools:
   ```powershell
   cmake --build --preset release # or debug
   ```
   Artifacts are emitted to `build/<preset>/out/{bin,lib}/<Config>/<x64|x86>`.

### 2.3 (Optional) Run clang-tidy

Use the dedicated preset to perform static analysis with clang-tidy. The preset reconfigures the project in a separate build tree so it does not disturb your normal build artifacts:

```powershell
cmake --preset clang-tidy
cmake --build --preset clang-tidy
```

Any diagnostics reported by clang-tidy are treated as errors and will cause the step to fail.

### 2.4 (Optional) Generate Documentation

After a successful build, you can produce the HTML documentation configured in `Doxyfile`:

```powershell
doxygen Doxyfile
```

The output is written to `docs/build/html/index.html`. Open that file in your browser to review the API documentation. Any warnings reported by Doxygen must be addressed because the configuration treats warnings as errors.

### 2.5 (Optional) Enforce Test Coverage

Sparkle includes a CMake target that builds the unit tests with Clang's coverage instrumentation and fails when the line coverage drops below the configured threshold (75% by default). Make sure LLVM/Clang is installed as described in section 1.5, then run:

```powershell
cmake --preset coverage-clang
cmake --build --preset coverage-clang
```

The second command compiles the library, executes the test suite, and enforces the coverage gate by invoking the custom `coverage` target. The run emits a human-readable summary at `build/coverage-clang/coverage/summary.txt`, which is also uploaded as a CI artifact. Adjust the minimum line coverage from the command line if required, for example:

```powershell
cmake --preset coverage-clang -DSPARKLE_COVERAGE_MIN_LINE_RATE=80
```

Running the build preset again will reuse the existing coverage configuration while applying the stricter threshold.

In addition to the summary text, the coverage run produces a full HTML report that highlights every instrumented region. You can open `build/coverage-clang/coverage/html/index.html` for the build-tree view, review the mirrored copy under `docs/build/coverage/index.html`, or browse the live site at `https://hyarius.github.io/HiddenSparkle/coverage/`.

## 3. Install Sparkle

Install the compiled headers, library, and CMake package configuration:

```powershell
cmake --install build/release # or build/debug
```

* By default, CMake installs to `C:\Program Files\Sparkle` on Windows.
* To customize the destination, pass a prefix during configuration or installation:
  ```powershell
  cmake --preset release -DCMAKE_INSTALL_PREFIX=C:\Dev\Sparkle
  cmake --install build/release --prefix C:\Dev\Sparkle
  ```
* The install tree contains:
  - `include\` – public headers plus the generated `spk_generated_resources.hpp`
  - `lib\` – static libraries
  - `lib\cmake\Sparkle\` – `SparkleConfig.cmake`, `SparkleTargets.cmake`, and version file

Once installed, CMake also registers the build tree with your user package registry so local projects can reference Sparkle without a manual install while you iterate.

## 4. Use Sparkle in Your Project

1. Make sure CMake can locate the Sparkle package:
   * If you used the default install prefix, CMake finds Sparkle automatically.
   * For a custom prefix, point CMake at the install directory:
     ```powershell
     cmake -S <project> -B <build> -DCMAKE_PREFIX_PATH="C:/Dev/Sparkle"
     ```
   * Alternatively, reference the package directory explicitly:
     ```powershell
     cmake -S <project> -B <build> -DSparkle_DIR="C:/Dev/Sparkle/lib/cmake/Sparkle"
     ```
2. In your project’s `CMakeLists.txt`, consume the package and link against the imported target:
   ```cmake
   cmake_minimum_required(VERSION 3.16)
   project(MyApp LANGUAGES CXX)

   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   find_package(Sparkle REQUIRED)

   add_executable(MyApp
       src/main.cpp
       # ...your sources...
   )

   target_link_libraries(MyApp PRIVATE Sparkle::Sparkle)
   ```
3. Sparkle exports its include directories, so once linked you can include headers directly:
   ```cpp
   #include <sparkle>
   ```

You are now ready to integrate Sparkle into your project

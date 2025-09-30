# Lab1 Linear Algebra - SDL3, ImGui, and GLEW Application

A cross-platform C++ application using SDL3, ImGui, and GLEW with automatic dependency management and build configuration for Linear Algebra operations.

## Features

- **SDL3 Integration**: Latest SDL3 with automatic download and setup
- **ImGui Integration**: Dear ImGui automatically downloaded from latest release
- **GLEW Integration**: OpenGL Extension Wrangler Library automatically downloaded
- **Cross-platform**: Support for Windows (Visual Studio) and Linux (GCC)
- **Multiple Backends**: OpenGL, Vulkan, Direct3D 11/12 support
- **Automatic Dependency Management**: All external dependencies (SDL3, ImGui, GLEW) are automatically downloaded
- **Multi-architecture**: x64 and x86 platform support
- **Latest Versions**: Automatically fetches the latest stable releases from GitHub

## Project Structure

```
Lab1_LinearAlgebra/
├── src/                    # Core source code (Linear Algebra implementations)
│   ├── DatasetIO.cpp
│   ├── LinAlg.cpp
│   ├── Matrix.cpp
│   └── Solve.cpp
├── include/                # Header files
│   ├── BenchConfig.hpp
│   ├── DatasetIO.hpp
│   ├── LinAlg.hpp
│   ├── Matrix.hpp
│   ├── OpsCounter.hpp
│   ├── Solve.hpp
│   └── Timer.hpp
├── app/                    # Application code (main GUI application)
│   └── main_app.cpp
├── bench/                  # Benchmarking code
│   ├── main_bench.cpp
│   └── generate_dataset.cpp
├── datasets/               # Binary datasets for testing
├── build/                  # Build system and dependencies
│   ├── premake5.exe        # Build tool (Windows)
│   ├── premake5            # Build tool (Linux)
│   ├── premake5.lua        # Build configuration
│   ├── build_files/        # Generated project files (auto-generated)
│   └── external/           # External dependencies (auto-downloaded)
│       ├── SDL3/           # SDL3 library (auto-downloaded)
│       ├── ImGui/          # Dear ImGui (auto-downloaded)
│       └── glew/           # GLEW library (auto-downloaded)
├── bin/                    # Compiled executables (auto-generated)
│   ├── Debug/
│   └── Release/
└── README.md
```

## Prerequisites

### Windows
- **Visual Studio 2022** (Community, Professional, or Enterprise)
- **Windows SDK** (latest)
- **Internet connection** (for automatic dependency download)

### Linux
- **GCC** with C++17 support
- **premake5** (included in build/ folder)
- **Development libraries**: X11, OpenGL
- **Internet connection** (for automatic dependency download)

## Quick Start

### Windows (Visual Studio)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/TheUnrealZaka/Lab1_LinearAlgebra.git
   cd Lab1_LinearAlgebra
   ```

2. **Execute the batch file to generate the .sln:**
   ```bash
   .\build-VisualStudio2022.bat
   ```
    It will automatically download SDL3, ImGui, and GLEW (latest versions) and generate the Visual Studio solution file.

### Linux

1. **Clone the repository:**
   ```bash
   git clone https://github.com/TheUnrealZaka/Lab1_LinearAlgebra.git
   cd Lab1_LinearAlgebra
   ```

2. **Generate build files:**
   ```bash
   cd build
   ./premake5 gmake2
   ```
    This will automatically download SDL3, ImGui, and GLEW (latest versions) and generate Makefiles.

3. **Build the project:**
   ```bash
   cd ..
   make config=debug_x64
   ```
    Note: The Linux build is experimental and may require additional system libraries for OpenGL and SDL3.

## SDL Backend Configuration

Choose your preferred graphics backend:

```bash
# OpenGL (default/recommended)
premake5 vs2022 --sdl_backend=opengl

# Vulkan
premake5 vs2022 --sdl_backend=vulkan

# Direct3D 11 (Windows only)
premake5 vs2022 --sdl_backend=d3d11

# Direct3D 12 (Windows only)
premake5 vs2022 --sdl_backend=d3d12

# Auto-detect best backend
premake5 vs2022 --sdl_backend=auto
```

## Platform and Architecture Options

### Platforms
- **x64**: 64-bit (default)
- **x86**: 32-bit

### Build Configurations
- **Debug**: Development build with symbols and assertions
- **Release**: Optimized production build

### Platform-specific Setup

#### Windows
The build system automatically:
- Links SDL3.lib
- Copies SDL3.dll to output directory
- Sets up Windows-specific libraries (winmm, gdi32, opengl32)
- Configures Unicode character set

## Development Workflow

### Adding Source Files
1. Add `.c` files to [`src/`](src/) directory
2. Add `.h` files to [`include/`](include/) directory
3. Rebuild project - files are automatically detected

### Project Structure for Development
```
src/
├── main.c              # Entry point
├── graphics/           # Graphics-related code
├── input/              # Input handling
├── audio/              # Audio processing
└── utils/              # Utility functions

include/
├── graphics.h
├── input.h
├── audio.h
└── utils.h
```

## SDL3 Features Available

This template provides access to all SDL3 capabilities:

- **Graphics**: 2D/3D rendering, multiple backend support
- **Input**: Keyboard, mouse, gamepad, touch
- **Audio**: Playback, recording, spatial audio
- **Window Management**: Multiple windows, fullscreen, borderless
- **File I/O**: Cross-platform file operations
- **Threading**: Thread creation and synchronization
- **Networking**: TCP/UDP sockets
- **Haptic Feedback**: Force feedback devices

## Example Usage

Basic SDL3 application structure:

```c
#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
    // Initialize SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("HOL - SDL3 App", 
                                         800, 600, 
                                         SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Main loop
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        
        // Your rendering code here
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

## Troubleshooting

### Dependency Issues
All dependencies (SDL3, ImGui, GLEW) are automatically downloaded when running premake5. If you encounter issues:

#### Automatic Download Failures
- **Network problems**: Check internet connection and firewall settings
- **API rate limiting**: GitHub API may be rate-limited; premake5 will use fallback versions
- **Manual download**:
  - SDL3: Download from [SDL3 releases](https://github.com/libsdl-org/SDL/releases) and extract to `build/external/SDL3/`
  - ImGui: Download from [ImGui releases](https://github.com/ocornut/imgui/releases) and extract to `build/external/ImGui/`
  - GLEW: Download from [GLEW releases](https://github.com/nigels-com/glew/releases) and extract to `build/external/glew/`

### Build Issues
#### Windows
- Ensure Visual Studio 2022 is installed
- Check Windows SDK version
- Run Visual Studio as Administrator if needed
- Delete `build/external/` and re-run premake5 to redownload dependencies

#### Linux
- Install required system libraries: `sudo apt-get install libglu1-mesa-dev libgl1-mesa-dev`
- The Linux build is experimental and primarily targets Windows

### Runtime Issues
- **Missing SDL3.dll**: Build system should copy automatically
- **Manual fix**: Copy from `build/external/SDL3/lib/x64/SDL3.dll` to `bin/Debug/`
- **Missing glew32.dll**: Build system should copy automatically
- **Manual fix**: Copy from `build/external/glew/bin/glew32.dll` to `bin/Debug/`

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Resources

- **SDL3 Documentation**: [SDL3 Wiki](https://wiki.libsdl.org/SDL3)
- **SDL3 Source**: [GitHub Repository](https://github.com/libsdl-org/SDL)
- **ImGui Documentation**: [ImGui GitHub](https://github.com/ocornut/imgui)
- **GLEW Documentation**: [GLEW Website](http://glew.sourceforge.net/)
- **Premake Documentation**: [Premake5 Docs](https://premake.github.io/docs/)
- **Community**: [SDL Discord](https://discord.com/invite/BwpFGBWsv8)

## Acknowledgments

- [SDL3](https://github.com/libsdl-org/SDL) - Simple DirectMedia Layer
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate Mode Graphical User Interface
- [GLEW](http://glew.sourceforge.net/) - OpenGL Extension Wrangler Library
- [Premake5](https://premake.github.io/) - Build configuration tool
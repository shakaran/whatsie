# Quick Reference: Building Whatsie with CMake

> Whatsie builds with **CMake** (Ninja generator recommended). There is no
> `Makefile` wrapper — all commands below drive CMake directly.

## TL;DR - Getting Started in 30 seconds

```bash
# Clone and navigate
git clone https://github.com/keshavbhatt/whatsie.git
cd whatsie

# Initialise the bundled libnotify-qt submodule
git submodule update --init --recursive

# Configure + build (Release mode)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/whatsie

# Install to system (optional)
cmake --install build          # uses the configured CMAKE_INSTALL_PREFIX
```

## Common Commands

### Configuring

```bash
# Release (optimized)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Debug
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Choose the install prefix at configure time (baked into the build)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$HOME/.local"

# Use a different build directory
cmake -S . -B mybuild -G Ninja -DCMAKE_BUILD_TYPE=Release
```

### Building

```bash
# Build (parallel, auto-detects cores)
cmake --build build --parallel

# Build with a fixed number of jobs
cmake --build build -j8

# Incremental rebuild after editing sources — same command
cmake --build build --parallel

# Or invoke Ninja directly inside the build dir
ninja -C build
```

### Running

```bash
# Run from the build directory
./build/whatsie

# Run from the system (after install, if prefix/bin is on PATH)
whatsie
```

### Installation & Uninstallation

```bash
# Install using the prefix chosen at configure time
cmake --install build

# Install system-wide to /usr (needs sudo; must have configured with that prefix)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build --parallel
sudo cmake --install build

# Uninstall (CMake records installed files in the manifest):
xargs rm -v < build/install_manifest.txt
```

> **Note:** `CMAKE_INSTALL_PREFIX` is resolved into absolute install paths at
> **configure** time. Passing `--prefix` to `cmake --install` after configuring
> with a different prefix will *not* take effect — reconfigure instead.

### Cleaning

```bash
# Remove all build artifacts
rm -rf build/
```

## Supported CMake Options

- `CMAKE_BUILD_TYPE`: `Debug` or `Release` (default: `Release`)
- `CMAKE_INSTALL_PREFIX`: installation prefix (default: `/usr/local`)
- `FLATPAK_BUILD`: skip spell-check dictionary compilation (`ON`/`OFF`, default: `OFF`)
- `-G <generator>`: build system generator (`Ninja` recommended, or `"Unix Makefiles"`)

```bash
# Flatpak build (no dictionary compilation)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DFLATPAK_BUILD=ON

# Use Unix Makefiles instead of Ninja
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# All options together
cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DFLATPAK_BUILD=OFF
```

## Environment Variables

```bash
# Point CMake at a non-standard Qt6 install
export CMAKE_PREFIX_PATH=/path/to/qt6/lib/cmake

# Speed up rebuilds with ccache
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## IDE Usage

### Qt Creator
1. File → Open File or Project
2. Select `CMakeLists.txt`
3. Qt Creator auto-configures
4. Select kit and build type in bottom-left
5. Ctrl+B to build

### VS Code
1. Install the "CMake Tools" extension
2. Open the folder containing `CMakeLists.txt`
3. Select a kit when prompted
4. F7 to build

### CLion
1. Open the project folder
2. CLion auto-detects `CMakeLists.txt`
3. Configure the build profile in preferences
4. Ctrl+Shift+F10 to build and run

## Troubleshooting Quick Tips

| Problem | Solution |
|---------|----------|
| Qt6 not found | `export CMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6` |
| Ninja not found | `sudo apt install ninja-build` |
| `notify-qt` submodule missing | `git submodule update --init --recursive` |
| Permission denied on install | Reconfigure with `-DCMAKE_INSTALL_PREFIX=$HOME/.local` (no sudo) |
| `qwebengine_convert_dict` missing | Ignore the warning or configure with `-DFLATPAK_BUILD=ON` |
| Build fails with C++ errors | Update the compiler: `sudo apt install build-essential` |

## Build Output Locations

```
build/
├── whatsie                      # Main executable
├── CMakeFiles/                  # CMake temporary files
├── cmake_install.cmake          # Installation script
├── install_manifest.txt         # List of installed files (after install)
├── compile_commands.json        # For IDE language servers
└── qtwebengine_dictionaries/    # Compiled spell-check dicts (if built)
```

## Performance Tips

- Use **Ninja** instead of Unix Makefiles (faster incremental builds)
- Use a **Release** build for production (`-DCMAKE_BUILD_TYPE=Release`)
- Parallelise explicitly with `-j`: `cmake --build build -j16`
- Use **ccache** for faster rebuilds: `-DCMAKE_CXX_COMPILER_LAUNCHER=ccache`

## Version & Build Info

```bash
./build/whatsie --version
./build/whatsie --build-info
```

## Development Workflow

1. **Initial setup**:
   ```bash
   git submodule update --init --recursive
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --parallel
   ```

2. **Iterative development**:
   ```bash
   # edit sources, then rebuild incrementally
   cmake --build build --parallel
   ./build/whatsie
   ```

3. **Before committing**:
   ```bash
   rm -rf build
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel
   ./build/whatsie     # final test
   ```

## Further Help

- [`CMAKE_MIGRATION.md`](CMAKE_MIGRATION.md) — detailed qmake → CMake migration guide
- `CMakeLists.txt` — source configuration (well-commented)

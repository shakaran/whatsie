# WhatSie

Feature rich WhatsApp web client based on Qt WebEngine for Linux Desktop

## Whatsie Key features

- Light and Dark Themes with automatic switching
- Customized Notifications & Native Notifications
- Keyboard Shortcuts
- BuiltIn download manager
- Mute Audio, Disable Notifications
- App Lock feature
- Hardware access permission manager
- Built in Spell Checker (with support for 31 Major languages)
- Other settings that let you control every aspect of WebApp like:
	+ Do not disturb mode
	+ Full view mode, lets you expand the main view to the full width of the window
	+ Ability to switch between Native & Custom notification
	+ Configurable notification popup timeout
	+ Mute all audio from Whatapp
	+ Disabling auto playback of media
	+ Minimize to tray on application start
	+ Toggle to enable single click hide to the system tray
	+ Switching download location
	+ Enable disable app lock on application start
	+ Auto-locking after a certain interval of time
	+ App lock password management
	+ Widget styling
	+ Configurable auto Theme switching based on day night time
	+ Configurable close button action
	+ Global App shortcuts
	+ Permission manager let you toggle camera mic and other hardware level permissions
	+ Configurable page zoom factor, switching based on window state maximized on normal 
	+ Configurable App User Agent
	+ Application Storage management, lets you clean residual cache and persistent data

## Command line options:
Comes with general CLI support, with a bunch of options that let you interact with already running instances of Whatsie.

Run: `whatsie -h` to see all supported options.

```
Usage: whatsie [options]
Feature rich WhatsApp web client based on Qt WebEngine

Options:
  -h, --help           Displays help on commandline options
  -v, --version        Displays version information.
  -b, --build-info     Shows detailed current build infomation
  -w, --show-window    Show main window of running instance of WhatSie
  -s, --open-settings  Opens Settings dialog in a running instance of WhatSie
  -l, --lock-app       Locks a running instance of WhatSie
  -i, --open-about     Opens About dialog in a running instance of WhatSie
  -t, --toggle-theme   Toggle between dark & light theme in a running instance
                       of WhatSie
  -r, --reload-app     Reload the app in a running instance of WhatSie
  -n, --new-chat       Open new chat prompt in a running instance of WhatSie
```

## Build from Source (Linux)

### Requirements
 - git, cmake >= 3.24, ninja-build
 - Qt6 >= 6.10 (qt6-base-dev, qt6-webengine-dev, qt6-positioning-dev)
 - C++17 compiler (GCC 7+, Clang 5+)
 - libx11-dev

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install cmake ninja-build qt6-base-dev qt6-webengine-dev \
    qt6-positioning-dev libx11-dev build-essential
```

**Fedora:**
```bash
sudo dnf install cmake ninja-build qt6-qtbase-devel qt6-qtwebengine-devel \
    qt6-qttools-devel libX11-devel gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S cmake ninja qt6-base qt6-webengine qt6-positioning
```

### Build & Run

The project uses CMake (with the Ninja generator) and bundles `libnotify-qt`
as a git submodule, so remember to initialise submodules after cloning.

```bash
git clone https://github.com/keshavbhatt/whatsie.git
cd whatsie
git submodule update --init --recursive

# Configure and build (Release)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run
./build/whatsie
```

### Install (Optional)

The install prefix is baked in at configure time, so set
`CMAKE_INSTALL_PREFIX` when configuring, then install.

```bash
# Install into your home (no sudo, ~/.local/bin is usually on PATH)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build --parallel
cmake --install build

# OR install system-wide to /usr (needs sudo)
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build --parallel
sudo cmake --install build
```

### Common Build Commands

```bash
# Debug build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Rebuild incrementally (after editing sources)
cmake --build build --parallel

# Build with a fixed number of jobs
cmake --build build -j8

# Clean build artifacts
rm -rf build

# Show version / build info
./build/whatsie --version
./build/whatsie --build-info
```

### Troubleshooting

| Problem | Solution |
|---------|----------|
| CMake not found | `sudo apt install cmake` |
| Qt6 not found | `sudo apt install qt6-base-dev qt6-webengine-dev` (or `export CMAKE_PREFIX_PATH=/usr/lib/cmake/Qt6`) |
| Ninja not found | `sudo apt install ninja-build` |
| `notify-qt` submodule missing | `git submodule update --init --recursive` |
| Permission denied on install | Reconfigure with `-DCMAKE_INSTALL_PREFIX=$HOME/.local` (no sudo) |

For detailed build instructions, see [`DOCS/BUILD_QUICK_REFERENCE.md`](DOCS/BUILD_QUICK_REFERENCE.md)
and [`DOCS/CMAKE_MIGRATION.md`](DOCS/CMAKE_MIGRATION.md).



## Install Whatsie on Linux Desktop

### On any snapd supported Linux distributions

 `snap install whatsie`

### On any Arch based Linux distribution
Using Arch User Repository (AUR), [AUR package for Whatsie](https://aur.archlinux.org/packages/whatsie-git) is maintained by [M0Rf30](https://github.com/M0Rf30)

 `yay -S whatsie-git`

## Screenshots (could be old)

![WhatSie for Linux Desktop Light Theme](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/1.jpg?raw=true)
![WhatSie for Linux Desktop Dark Theme](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/2.jpg?raw=true)
![WhatSie for Linux Desktop Setting module](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/4.jpg?raw=true)
![WhatSie for Linux Desktop App Lock screen](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/3.jpg?raw=true)
![WhatSie for Linux Desktop Shortcuts & Permissions](https://github.com/keshavbhatt/whatsie/blob/main/screenshots/5.jpg?raw=true)

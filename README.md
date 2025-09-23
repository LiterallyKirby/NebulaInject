# Nebula ðŸš€

**License:** TBD â€¢ **Platform:** Linux â€¢ **Language:** C++

Nebula is a C++ Minecraft injection client designed for Linux, with Windows support planned for a future release. It borrows some core code and concepts from the Phantom project.

---

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Building](#building)
- [Dependencies](#dependencies)
- [Usage](#usage)
- [Examples](#examples)
- [Screenshots](#screenshots)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)

---

## Introduction

Nebula is an open-source injection client for Minecraft, built in C++. It allows advanced users to modify the Minecraft client through injection, primarily targeting Linux users.

---

## Features âœ…

- Linux support (stable)
- Windows support (planned)
- Custom injection for Minecraft
- Integration with Phantom Injector
- Standalone modern injector (in development)

---

## Building ðŸ› ï¸

Ensure all dependencies are installed, then navigate to the root of the repository and run:

```bash
./build.sh
```

If the build script fails, refer to the [Phantom repository build instructions](https://github.com/Phantom/Phantom).

---

## Dependencies ðŸ“¦

- **Java 8**: Required for Minecraft.
- **CMake**: Cross-platform build tool.
- **Minecraft**: The target game client.

---

## Usage ðŸŽ®

Currently, Nebula relies on an external tool for injection. You can use **Phantom Injector** to load the Nebula client into Minecraft.

A new, standalone injector with a modern UI is under development.

---

## Examples ðŸ’»

Inject Nebula into Minecraft using Phantom Injector:

```bash
# Launch Phantom Injector
./phantom-injector

# Select the Nebula client
# Choose the target Minecraft process
# Inject
```

Building Nebula from source:

```bash
git clone https://github.com/YourUsername/Nebula.git
cd Nebula
./build.sh
```

---

## Screenshots ðŸ–¼ï¸

![Nebula Screenshot](./screenshots/example.png)  
*Caption: Nebula injected into Minecraft on Linux*

---

## Troubleshooting âš ï¸

### Wayland/Hyprland Users

If using a Wayland-based display server such as Hyprland and you encounter mouse input issues:

1. Install and run **xwayland**.
2. Ensure your mouse is configured as the **xwayland pointer**.

---

## Contributing ðŸ¤

Contributions are welcome! Please check the `CONTRIBUTING.md` (coming soon) for guidelines on pull requests and code of conduct.

---

## License ðŸ“„

License information will be added here.

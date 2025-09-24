---

# Nebula

**License:** TBD • **Platform:** Linux (Windows support planned) • **Language:** C++

Nebula is a C++ Minecraft injection client for Linux, inspired by the Phantom project. It enables advanced users to modify Minecraft through injection, with future plans for a standalone, modern injector and Windows support.

---

## Table of Contents

* [Introduction](#introduction)
* [Features](#features)
* [Building](#building)
* [Dependencies](#dependencies)
* [Usage](#usage)
* [Examples](#examples)
* [Screenshots](#screenshots)
* [Troubleshooting](#troubleshooting)
* [Contributing](#contributing)
* [License](#license)

---

## Introduction

Nebula is an open-source injection client for Minecraft written in C++. It is designed for Linux but has plans to expand to Windows. Nebula leverages core concepts and code from the Phantom project to deliver powerful injection features for modding Minecraft.

---

## Features

* Stable Linux support
* Planned Windows support
* Custom Minecraft injection
* Phantom Injector integration
* Modern standalone injector (in development)

---

## Building

1. Ensure all dependencies are installed (see [Dependencies](#dependencies)).
2. Navigate to the root of the repository:

   ```bash
   cd Nebula
   ```
3. Run the build script:

   ```bash
   ./build.sh
   ```

If the build fails, refer to the [Phantom repository build instructions](https://github.com/Phantom/Phantom).

---

## Dependencies

* **Java 8** — Required for Minecraft
* **CMake** — Cross-platform build tool
* **Minecraft** — The target client

---

## Usage

Nebula currently relies on an external tool for injection. Use **Phantom Injector** to load Nebula into Minecraft.

A standalone injector with a modern UI is in progress.

**Important:** Always inject using the `.so` file in `build-inject`, unless you are using address sanitizing.
If so, add the following to your wrapper commands in Prism Launcher:

```bash
sh -c 'env LD_PRELOAD=$(find /usr/lib /usr/lib64 -name "libasan.so*" | head -n 1) "$@"' sh
```

---

## Examples

### Injecting Nebula into Minecraft

```bash
./phantom-injector
```

* Select the Nebula client
* Choose your target Minecraft process
* Click **Inject**

### Building Nebula from source

```bash
git clone https://github.com/YourUsername/Nebula.git
cd Nebula
./build.sh
```

---

## Screenshots

![Nebula Screenshot](./screenshots/example.png)
*Nebula injected into Minecraft on Linux.*

---

## Troubleshooting

### Wayland / Hyprland Users

If you encounter mouse input issues:

1. Install and run **xwayland**.
2. Configure your mouse as the **xwayland pointer**.

⚠️ Nebula is currently locked to vanilla 1.8.9 due to Wayland title handling quirks — Hyprland titles default to `"Hyprland :D"` instead of the actual game title.

---

## Contributing

Contributions are welcome! Please refer to `CONTRIBUTING.md` (coming soon) for pull request guidelines and code of conduct.

---

## License

License information will be added here.

---

✅ Improvements made:
* make mappings slightly better, need to re-add everything






---


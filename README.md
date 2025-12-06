# Antistatic Loader

A lightweight cross-platform launcher for the Antistatic game that runs the Node.js-based game without displaying console windows.

## Purpose

This loader provides a seamless experience for running Antistatic by:
- Launching the game without visible terminal/console windows (Windows GUI mode)
- Running Node.js with security flags enabled (`--disallow-code-generation-from-strings`)
- Providing debug logging for troubleshooting
- Passing through command line arguments to the game
- Properly handling process lifecycle and exit codes
- Cross-platform support (Windows and Linux)

## Requirements

### Common Requirements
- Python 3.6+ (for building)
- Node.js runtime (`node.exe` on Windows, `node` on Linux)
- Game files at `./app/dist/src/engine.js`

### Platform-Specific Requirements

**Windows:**
- Visual Studio Build Tools or Visual Studio with C++ development tools (MSVC), OR
- MinGW-w64 with GCC, OR
- Clang for Windows

**Linux:**
- GCC (g++) or Clang (clang++)
- Build essentials: `sudo apt-get install build-essential` (Ubuntu/Debian)

## Building

### Quick Start

Run the Python build script:

```bash
python3 build.py
```

The script automatically detects your platform and available compiler, then builds the appropriate executable.

### Manual Build (Advanced)

**Windows with MSVC:**
```powershell
.\build.ps1
```

**Windows with GCC/Clang:**
```bash
g++ src/Antistatic.cpp -o bin/Antistatic.exe -std=c++17 -Wall -Wextra -O2
```

**Linux:**
```bash
g++ src/Antistatic.cpp -o bin/antistatic -std=c++17 -Wall -Wextra -O2 -pthread
```

## Usage

Place the compiled executable in the same directory as:
- `node.exe` (Windows) or `node` (Linux) - Node.js runtime
- `app/dist/src/engine.js` - The game entry point

Run the launcher:

**Windows:**
```
Antistatic.exe [optional game arguments]
```

**Linux:**
```
./antistatic [optional game arguments]
```

Any command line arguments will be passed through to the Node.js game.

## Build Output

- **Windows:** `bin/Antistatic.exe` (GUI application, no console window)
- **Linux:** `bin/antistatic` (standard executable)

## Debugging

The launcher creates a `debug.txt` file in the current directory with diagnostic information:
- Startup confirmation
- Command line arguments
- Node.js process creation status
- Process ID and exit code
- Detailed error messages if launch fails

## Architecture

The launcher uses platform-specific APIs to launch Node.js:
- **Windows:** Uses `CreateProcess` API with GUI subsystem to avoid console windows
- **Linux:** Uses `fork`/`exec` pattern for process creation

Both implementations:
- Use RAII for automatic resource cleanup
- Properly propagate exit codes
- Provide detailed error logging
- Support command-line argument forwarding

## Security

The launcher enforces security by default:
- Always runs Node.js with `--disallow-code-generation-from-strings` flag
- Uses modern C++17 features for memory safety
- Compiled with security flags (DEP, ASLR, CFG on Windows)

## License

See [LICENSE](LICENSE) file for details.

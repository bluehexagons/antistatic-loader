# Antistatic Loader

A lightweight Windows launcher for the Antistatic game that runs the Node.js-based game without displaying console windows.

## Purpose

This loader provides a seamless experience for running Antistatic by:
- Launching the game without any visible terminal/console windows
- Running Node.js with security flags enabled (`--disallow-code-generation-from-strings`)
- Providing debug logging for troubleshooting
- Passing through command line arguments to the game
- Properly handling process lifecycle and exit codes

## Requirements

- Windows operating system
- Visual Studio Build Tools or Visual Studio with C++ development tools
- Node.js executable (`node.exe`) in the same directory as the launcher
- Game files at `./app/dist/src/engine.js`

## Building

Run the build script using PowerShell:

```powershell
.\build.ps1
```

This will:
1. Create `build` and `bin` directories
2. Compile the resource file (icon)
3. Compile the C++ source with security features enabled
4. Output `bin\Antistatic.exe`

### Build Flags

The build uses several security and optimization flags:
- `/std:c++17` - C++17 standard
- `/W4 /WX` - High warning level, treat warnings as errors
- `/O2` - Maximum optimization
- `/analyze` - Static code analysis
- `/GUARD:CF` - Control Flow Guard for exploit mitigation
- `/NXCOMPAT` - Data Execution Prevention
- `/DYNAMICBASE` - Address Space Layout Randomization (ASLR)

## Usage

Place `Antistatic.exe` in the same directory as:
- `node.exe` - Node.js runtime
- `app/dist/src/engine.js` - The game entry point

Run the launcher:
```
Antistatic.exe [optional game arguments]
```

Any command line arguments will be passed through to the Node.js game.

## Debugging

The launcher creates a `debug.txt` file in the current directory with diagnostic information:
- Startup confirmation
- Command line arguments
- Node.js process creation status
- Process ID and exit code
- Detailed error messages if launch fails

## License

See [LICENSE](LICENSE) file for details.

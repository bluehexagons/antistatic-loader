# Antistatic Loader

Launches the Antistatic Node.js game without showing console windows.

## Installation

Download pre-built binaries from [Releases](https://github.com/bluehexagons/antistatic-loader/releases):
- `Antistatic-windows-amd64.exe` - Windows
- `antistatic-linux-amd64` - Linux x86_64
- `antistatic-linux-arm64` - Raspberry Pi 4/5

## Building

```bash
python3 build.py
```

Requirements:
- Python 3.6+
- C++ compiler (MSVC/GCC/Clang)

## Usage

Place the executable next to:
- `node.exe` (Windows) or `node` (Linux)
- `app/dist/src/engine.js`

Run: `./antistatic [arguments]`

Arguments are passed to the Node.js game. Debug output is written to `debug.txt`.

## License

See [LICENSE](LICENSE) file.

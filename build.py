#!/usr/bin/env python3
"""
Cross-platform build script for Antistatic Loader
Supports Windows (MSVC) and Linux (GCC/Clang)
"""

import sys
import os
import platform
import subprocess
import shutil
from pathlib import Path


class BuildConfig:
    """Build configuration for different platforms"""
    
    def __init__(self):
        self.system = platform.system()
        self.build_dir = Path("build")
        self.bin_dir = Path("bin")
        self.src_file = Path("src/Antistatic.cpp")
        
        if self.system == "Windows":
            self.output_name = "Antistatic.exe"
            self.resource_file = Path("antistatic.rc")
        else:
            self.output_name = "antistatic"
            self.resource_file = None
    
    def get_compiler(self):
        """Detect available compiler"""
        if self.system == "Windows":
            if shutil.which("cl"):
                return "msvc"
            elif shutil.which("g++"):
                return "gcc"
            elif shutil.which("clang++"):
                return "clang"
        else:  # Linux/Unix
            if shutil.which("g++"):
                return "gcc"
            elif shutil.which("clang++"):
                return "clang"
        
        return None


class Builder:
    """Handles compilation for different compilers"""
    
    def __init__(self, config):
        self.config = config
        self.compiler = config.get_compiler()
        
        if not self.compiler:
            raise RuntimeError("No suitable compiler found. Install MSVC, GCC, or Clang.")
    
    def create_directories(self):
        """Create build and bin directories"""
        print("Creating build directories...")
        self.config.build_dir.mkdir(exist_ok=True)
        self.config.bin_dir.mkdir(exist_ok=True)
    
    def compile_resources_msvc(self):
        """Compile Windows resource file"""
        if not self.config.resource_file or not self.config.resource_file.exists():
            return None
        
        print("Compiling resources...")
        res_output = self.config.build_dir / "antistatic.res"
        
        try:
            subprocess.run(
                ["rc", f"/fo{res_output}", str(self.config.resource_file)],
                check=True,
                capture_output=True,
                text=True
            )
            return res_output
        except subprocess.CalledProcessError as e:
            print(f"ERROR: Resource compilation failed")
            print(e.stderr)
            sys.exit(1)
    
    def build_msvc(self):
        """Build using MSVC (Windows)"""
        print("Building with MSVC...")
        
        # Compile resources if available
        res_file = self.compile_resources_msvc()
        
        # Build compiler arguments
        compiler_args = [
            "cl",
            f"/Fo{self.config.build_dir / 'antistatic.obj'}",
            f"/Fe{self.config.bin_dir / self.config.output_name}",
            str(self.config.src_file),
        ]
        
        if res_file:
            compiler_args.append(str(res_file))
        
        compiler_args.extend([
            "/std:c++17",
            "/EHsc",
            "/W4",
            "/WX",
            "/O2",
            "/DNDEBUG",
            "/link",
            "/SUBSYSTEM:WINDOWS",
            "/RELEASE",
            "/GUARD:CF",
            "/NXCOMPAT",
            "/DYNAMICBASE",
        ])
        
        try:
            subprocess.run(compiler_args, check=True)
        except subprocess.CalledProcessError:
            print("ERROR: Compilation failed")
            sys.exit(1)
    
    def build_gcc_clang(self):
        """Build using GCC or Clang (Linux/Unix)"""
        compiler_cmd = "g++" if self.compiler == "gcc" else "clang++"
        print(f"Building with {compiler_cmd}...")
        
        output_path = self.config.bin_dir / self.config.output_name
        
        # Build compiler arguments
        compiler_args = [
            compiler_cmd,
            str(self.config.src_file),
            "-o", str(output_path),
            "-std=c++17",
            "-Wall",
            "-Wextra",
            "-Werror",
            "-O2",
            "-DNDEBUG",
        ]
        
        # Platform-specific flags
        if self.config.system == "Linux":
            compiler_args.extend(["-pthread"])
        
        try:
            subprocess.run(compiler_args, check=True)
        except subprocess.CalledProcessError:
            print("ERROR: Compilation failed")
            sys.exit(1)
    
    def build(self):
        """Execute the build"""
        self.create_directories()
        
        if self.compiler == "msvc":
            self.build_msvc()
        else:
            self.build_gcc_clang()
        
        print(f"✓ Build successful! Output: {self.config.bin_dir / self.config.output_name}")


def main():
    """Main entry point"""
    print("Antistatic Loader Build Script")
    print(f"Platform: {platform.system()}")
    print()
    
    try:
        config = BuildConfig()
        builder = Builder(config)
        builder.build()
        return 0
    except Exception as e:
        print(f"ERROR: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())

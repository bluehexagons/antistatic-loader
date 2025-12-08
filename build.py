#!/usr/bin/env python3
"""
Cross-platform build script for Antistatic Loader
Supports Windows (MSVC) and Linux (GCC/Clang)
Supports x86_64 and ARM architectures (including Raspberry Pi, with future Android/iOS support)
"""

import sys
import os
import platform
import subprocess
import shutil
from pathlib import Path


class BuildConfig:
    """Build configuration for different platforms and architectures"""
    
    def __init__(self):
        self.system = platform.system()
        self.machine = platform.machine().lower()
        self.build_dir = Path("build")
        self.bin_dir = Path("bin")
        self.src_file = Path("src/Antistatic.cpp")
        
        # Detect architecture
        self.is_arm = self._detect_arm()
        self.is_64bit = self._detect_64bit()
        
        if self.system == "Windows":
            self.output_name = "Antistatic.exe"
            self.resource_file = Path("antistatic.rc")
        else:
            self.output_name = "antistatic"
            self.resource_file = None
    
    def _detect_arm(self):
        """Detect if running on ARM architecture"""
        arm_indicators = ['arm', 'aarch64', 'arm64']
        return any(indicator in self.machine for indicator in arm_indicators)
    
    def _detect_64bit(self):
        """Detect if running on 64-bit architecture"""
        return '64' in self.machine or self.machine in ['aarch64', 'arm64']
    
    def get_arch_description(self):
        """Get human-readable architecture description"""
        if self.is_arm:
            bits = "64-bit" if self.is_64bit else "32-bit"
            return f"ARM {bits} ({self.machine})"
        else:
            bits = "64-bit" if self.is_64bit else "32-bit"
            return f"x86 {bits} ({self.machine})"
    
    def get_compiler(self):
        """Detect available compiler"""
        if self.system == "Windows":
            if shutil.which("cl"):
                return "msvc"
            elif shutil.which("g++"):
                return "gcc"
            elif shutil.which("clang++"):
                return "clang"
        else:  # Linux/Unix (including Android in future)
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
            "/analyze",
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
        """Build using GCC or Clang (Linux/Unix/ARM)"""
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
            
            # ARM-specific optimizations for Raspberry Pi 4+
            if self.config.is_arm:
                if self.config.is_64bit:
                    # ARMv8 (Raspberry Pi 4/5, modern ARM)
                    # Use generic ARMv8 flags for broad compatibility
                    compiler_args.extend([
                        "-march=armv8-a",
                        "-mtune=cortex-a72",  # Raspberry Pi 4/5 compatible
                    ])
                else:
                    # ARMv7 (older Raspberry Pi)
                    compiler_args.extend([
                        "-march=armv7-a",
                        "-mfpu=neon-vfpv4",
                        "-mfloat-abi=hard",
                    ])
        
        # Future: Android NDK support
        # elif self.config.system == "Android":
        #     compiler_args.extend(["-fPIE", "-pie"])
        #     if self.config.is_arm:
        #         compiler_args.extend(["-march=armv7-a", "-mfloat-abi=softfp"])
        
        # Future: iOS/macOS ARM (Apple Silicon) support  
        # elif self.config.system == "Darwin":
        #     if self.config.is_arm:
        #         compiler_args.extend(["-arch", "arm64"])
        
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
        print(f"Architecture: {config.get_arch_description()}")
        print()
        
        builder = Builder(config)
        builder.build()
        return 0
    except Exception as e:
        print(f"ERROR: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Cross-platform build script for Antistatic Loader"""

import sys
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
        
        self.is_arm = self._detect_arm()
        self.is_64bit = self._detect_64bit()
        
        if self.system == "Windows":
            self.output_name = "Antistatic.exe"
            self.resource_file = Path("antistatic.rc")
        else:
            self.output_name = "antistatic"
            self.resource_file = None
    
    def _detect_arm(self):
        arm_patterns = ['aarch64', 'arm64', 'armv8', 'armv7', 'armv6']
        if self.machine in arm_patterns:
            return True
        return any(self.machine.startswith(p) for p in arm_patterns)
    
    def _detect_64bit(self):
        return '64' in self.machine or self.machine in ['aarch64', 'arm64']
    
    def get_arch_description(self):
        arch_type = "ARM" if self.is_arm else "x86"
        bits = "64-bit" if self.is_64bit else "32-bit"
        return f"{arch_type} {bits} ({self.machine})"
    
    def get_compiler(self):
        if self.system == "Windows":
            for compiler in ["cl", "g++", "clang++"]:
                if shutil.which(compiler):
                    return "msvc" if compiler == "cl" else compiler.replace("++", "")
        else:
            for compiler in ["g++", "clang++"]:
                if shutil.which(compiler):
                    return compiler.replace("++", "")
        return None


class Builder:
    """Handles compilation for different compilers"""
    
    def __init__(self, config):
        self.config = config
        self.compiler = config.get_compiler()
        if not self.compiler:
            raise RuntimeError("No compiler found. Install MSVC, GCC, or Clang.")
    
    def create_directories(self):
        print("Creating build directories...")
        self.config.build_dir.mkdir(exist_ok=True)
        self.config.bin_dir.mkdir(exist_ok=True)
    
    def compile_resources_msvc(self):
        if not self.config.resource_file or not self.config.resource_file.exists():
            return None
        
        print("Compiling resources...")
        res_output = self.config.build_dir / "antistatic.res"
        try:
            subprocess.run(
                ["rc", f"/fo{res_output}", str(self.config.resource_file)],
                check=True, capture_output=True, text=True
            )
            return res_output
        except subprocess.CalledProcessError as e:
            print("ERROR: Resource compilation failed")
            if e.stderr:
                print(e.stderr)
            sys.exit(1)
    
    def build_msvc(self):
        print("Building with MSVC...")
        res_file = self.compile_resources_msvc()
        
        compiler_args = [
            "cl",
            f"/Fo{self.config.build_dir / 'antistatic.obj'}",
            f"/Fe{self.config.bin_dir / self.config.output_name}",
            str(self.config.src_file),
        ]
        
        if res_file:
            compiler_args.append(str(res_file))
        
        compiler_args.extend([
            "/std:c++17", "/EHsc", "/W4", "/WX", "/O2", "/DNDEBUG",
            "/link", "/SUBSYSTEM:WINDOWS", "/RELEASE",
            "/GUARD:CF", "/NXCOMPAT", "/DYNAMICBASE",
        ])
        
        try:
            subprocess.run(compiler_args, check=True)
        except subprocess.CalledProcessError:
            print("ERROR: Compilation failed")
            sys.exit(1)
    
    def build_gcc_clang(self):
        compiler_cmd = "g++" if self.compiler == "gcc" else "clang++"
        print(f"Building with {compiler_cmd}...")
        
        output_path = self.config.bin_dir / self.config.output_name
        compiler_args = [
            compiler_cmd, str(self.config.src_file),
            "-o", str(output_path),
            "-std=c++17", "-Wall", "-Wextra", "-Werror",
            "-O2", "-DNDEBUG",
        ]
        
        if self.config.system == "Linux":
            compiler_args.append("-pthread")
            
            if self.config.is_arm:
                if self.config.is_64bit:
                    compiler_args.append("-march=armv8-a")
                    try:
                        with open('/proc/cpuinfo', 'r') as f:
                            cpuinfo = f.read()
                            if 'BCM2711' in cpuinfo or 'Cortex-A72' in cpuinfo:
                                compiler_args.append("-mtune=cortex-a72")
                            elif 'BCM2712' in cpuinfo or 'Cortex-A76' in cpuinfo:
                                compiler_args.append("-mtune=cortex-a76")
                    except (FileNotFoundError, IOError):
                        pass
                else:
                    compiler_args.extend([
                        "-march=armv7-a", "-mfpu=neon-vfpv4", "-mfloat-abi=hard"
                    ])
        
        try:
            subprocess.run(compiler_args, check=True)
        except subprocess.CalledProcessError:
            print("ERROR: Compilation failed")
            sys.exit(1)
    
    def build(self):
        self.create_directories()
        if self.compiler == "msvc":
            self.build_msvc()
        else:
            self.build_gcc_clang()
        print(f"✓ Build successful: {self.config.bin_dir / self.config.output_name}")


def main():
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

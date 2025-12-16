#!/usr/bin/env python3
import sys
import platform
import subprocess
import shutil
from pathlib import Path


class BuildConfig:
    def __init__(self):
        self.system = platform.system()
        self.machine = platform.machine().lower()
        self.build_dir = Path("build")
        self.bin_dir = Path("bin")
        self.src_file = Path("src/Antistatic.cpp")
        
        self.is_arm = self._detect_arm()
        self.is_64bit = '64' in self.machine or self.machine in ['aarch64', 'arm64']
        
        if self.system == "Windows":
            self.output_name = "Antistatic.exe"
            self.resource_file = Path("antistatic.rc")
        else:
            self.output_name = "antistatic"
            self.resource_file = None
    
    def _detect_arm(self):
        arm_patterns = ['aarch64', 'arm64', 'armv8', 'armv7', 'armv6']
        return self.machine in arm_patterns or any(self.machine.startswith(p) for p in arm_patterns)
    
    def get_compiler(self):
        if self.system == "Windows":
            for c in ["cl", "g++", "clang++"]:
                if shutil.which(c):
                    return "msvc" if c == "cl" else c.replace("++", "")
        else:
            for c in ["g++", "clang++"]:
                if shutil.which(c):
                    return c.replace("++", "")
        return None


class Builder:
    def __init__(self, config):
        self.config = config
        self.compiler = config.get_compiler()
        if not self.compiler:
            raise RuntimeError("No compiler found")
    
    def build(self):
        self.config.build_dir.mkdir(exist_ok=True)
        self.config.bin_dir.mkdir(exist_ok=True)
        
        if self.compiler == "msvc":
            self._build_msvc()
        else:
            self._build_gcc()
        
        print(f"✓ {self.config.bin_dir / self.config.output_name}")
    
    def _build_msvc(self):
        res = None
        if self.config.resource_file and self.config.resource_file.exists():
            res = self.config.build_dir / "antistatic.res"
            subprocess.run(["rc", f"/fo{res}", str(self.config.resource_file)], check=True, capture_output=True)
        
        args = ["cl", f"/Fo{self.config.build_dir / 'antistatic.obj'}", 
                f"/Fe{self.config.bin_dir / self.config.output_name}", str(self.config.src_file)]
        if res:
            args.append(str(res))
        
        args.extend(["/std:c++17", "/EHsc", "/W4", "/WX", "/O2", "/DNDEBUG",
                     "/link", "/SUBSYSTEM:WINDOWS", "/RELEASE", "/GUARD:CF", "/NXCOMPAT", "/DYNAMICBASE"])
        subprocess.run(args, check=True)
    
    def _build_gcc(self):
        cmd = "g++" if self.compiler == "gcc" else "clang++"
        args = [cmd, str(self.config.src_file), "-o", str(self.config.bin_dir / self.config.output_name),
                "-std=c++17", "-Wall", "-Wextra", "-Werror", "-O2", "-DNDEBUG"]
        
        if self.config.system == "Linux":
            args.append("-pthread")
            if self.config.is_arm:
                if self.config.is_64bit:
                    args.append("-march=armv8-a")
                    try:
                        cpuinfo = open('/proc/cpuinfo').read()
                        if 'BCM2711' in cpuinfo or 'Cortex-A72' in cpuinfo:
                            args.append("-mtune=cortex-a72")
                        elif 'BCM2712' in cpuinfo or 'Cortex-A76' in cpuinfo:
                            args.append("-mtune=cortex-a76")
                    except:
                        pass
                else:
                    args.extend(["-march=armv7-a", "-mfpu=neon-vfpv4", "-mfloat-abi=hard"])
        
        subprocess.run(args, check=True)


if __name__ == "__main__":
    try:
        config = BuildConfig()
        Builder(config).build()
    except Exception as e:
        print(f"ERROR: {e}")
        sys.exit(1)

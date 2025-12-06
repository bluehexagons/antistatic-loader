# Build script for Antistatic Loader
# Requires Visual Studio Build Tools or Visual Studio with C++ development tools

# Exit on error
$ErrorActionPreference = "Stop"

# Build configuration
$CppStandard = "/std:c++17"
$WarningFlags = "/W4", "/WX"
$OptimizationFlags = "/O2"
$AnalysisFlags = "/analyze"
$SubsystemFlag = "/SUBSYSTEM:WINDOWS"
$LinkFlags = "/RELEASE", "/GUARD:CF", "/NXCOMPAT", "/DYNAMICBASE"

Write-Host "Building Antistatic Loader..." -ForegroundColor Cyan

# Create build directories
Write-Host "Creating build directories..." -ForegroundColor Gray
New-Item -Path build -ItemType Directory -Force | Out-Null
New-Item -Path bin -ItemType Directory -Force | Out-Null

# Compile resource file
Write-Host "Compiling resources..." -ForegroundColor Gray
rc /fo"build\antistatic.res" antistatic.rc
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Resource compilation failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

# Compile C++ source with optimizations and security features
Write-Host "Compiling C++ source..." -ForegroundColor Gray

# Build compiler and linker argument lists
$CompilerArgs = @(
    "/Fo`"build\antistatic.obj`""
    "/Fe`"bin\Antistatic.exe`""
    "src\Antistatic.cpp"
    "build\antistatic.res"
    $CppStandard
    "/EHsc"
    $WarningFlags
    $OptimizationFlags
    $AnalysisFlags
)

$LinkerArgs = @(
    "/link"
    $SubsystemFlag
    $LinkFlags
)

# Execute compilation using proper array splatting
& cl @CompilerArgs @LinkerArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Compilation failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Build successful! Output: bin\Antistatic.exe" -ForegroundColor Green

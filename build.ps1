rc /fo"build\antistatic.res" antistatic.rc
cl /Fo"build\antistatic.obj" /Fe"bin\Antistatic.exe" "src\Antistatic.cpp" "build\antistatic.res" /std:c++17 /EHsc /analyze /link /SUBSYSTEM:WINDOWS /RELEASE /GUARD:CF /NXCOMPAT

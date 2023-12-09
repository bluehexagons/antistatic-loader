rc antistatic.rc
cl Antistatic.cpp antistatic.res /std:c++17 /EHsc /analyze /link /SUBSYSTEM:WINDOWS /RELEASE /GUARD:CF /NXCOMPAT

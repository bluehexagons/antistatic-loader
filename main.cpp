#include <windows.h>
#include <iostream>

int main() {
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::cout << "Launching" << std::endl;

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    LPSTR command = "node.exe --disallow-code-generation-from-strings ./app/dist/src/engine.js";

    if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cout << "Created process" << std::endl;
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::cout << "Closed process" << std::endl;
    } else {
        std::cout << "Unable to create process" << std::endl;
    }

    return 0;
}

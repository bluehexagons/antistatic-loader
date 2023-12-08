#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>

int main() {
    std::cout << "Running main() for some reason? This won't do anything." << std::endl;
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::string logFile = "./debug.txt";
    std::ofstream log(logFile);
    bool writeLog = log.good();

    if (writeLog) log << "Launching" << std::endl;

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    std::string nodeLocation = "./node.exe";

    std::ifstream nodeStream(nodeLocation);
    if (!nodeStream.good()) {
        if (writeLog) {
            log << "Unable to find nodejs at " << nodeLocation << std::endl;
            log.close();
        }

        return 0;
    }

    std::string commandArgs = " --disallow-code-generation-from-strings ./app/dist/src/engine.js";

    std::string commandString = nodeLocation + commandArgs;

    char *command = new char[commandString.length() + 1];
    strcpy(command, commandString.c_str());

    if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        if (writeLog) log << "Created nodejs process" << std::endl;
        
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (writeLog) log << "Closed nodejs process" << std::endl;
    } else {
        if (writeLog) log << "Unable to create nodejs process" << std::endl;
    }

    if (writeLog) {
        log.close();
    }

    return 0;
}

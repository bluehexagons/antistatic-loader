#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>

int main() {
    std::cout << "Running main() for some reason? This won't do anything." << std::endl;
    return 0;
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // For now, save a log in debug.txt if possible
    std::string logFile = "./debug.txt";
    std::ofstream log(logFile);
    bool writeLog = log.good();

    if (writeLog) log << "Launching" << std::endl;

    std::string nodeLocation = "./node.exe";

    // Make sure node.exe is actually there
    std::ifstream nodeStream(nodeLocation);
    if (!nodeStream.good()) {
        if (writeLog) {
            log << "Unable to find nodejs at " << nodeLocation << std::endl;
            log.close();
        }

        return 0;
    }

    // For added security, we disallow code generation from strings when running the game
    std::string commandArgs = " --disallow-code-generation-from-strings ./app/dist/src/engine.js";

    std::string commandString = nodeLocation + commandArgs;

    char *command = new char[commandString.length() + 1];
    strcpy(command, commandString.c_str());

    // Variables to hold the output from CreateProcess
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    // Attempt to launch the game using the generated command
    if (CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        if (writeLog) log << "Created nodejs process" << std::endl;
        
        // Chill until node.exe exits, then cleanup
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (writeLog) log << "Closed nodejs process" << std::endl;
    } else {
        if (writeLog) log << "Unable to create nodejs process" << std::endl;
    }

    // Cleanup
    if (writeLog) {
        log.close();
    }

    return 0;
}

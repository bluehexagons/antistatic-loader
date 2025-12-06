#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

// This main() is not used when compiled as a Windows GUI application
// The entry point is WinMain below
int main() {
    std::cout << "This launcher must be built as a Windows GUI application." << std::endl;
    return 0;
}

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    // Save debug log if possible
    const std::string logFile = "./debug.txt";
    std::ofstream log(logFile);
    const bool writeLog = log.good();

    if (writeLog) {
        log << "Antistatic Loader Starting..." << std::endl;
        log << "Command line: " << (lpCmdLine ? lpCmdLine : "(none)") << std::endl;
    }

    const std::string nodeLocation = "./node.exe";

    // Verify node.exe exists
    std::ifstream nodeStream(nodeLocation);
    if (!nodeStream.good()) {
        if (writeLog) {
            log << "ERROR: Unable to find Node.js at " << nodeLocation << std::endl;
            log.close();
        }
        return 1;
    }
    nodeStream.close();

    // Build command line: node.exe with security flags, game script, and any passed arguments
    // For added security, we disallow code generation from strings when running the game
    std::string commandArgs = " --disallow-code-generation-from-strings ./app/dist/src/engine.js";
    
    // Append any command line arguments passed to the loader
    if (lpCmdLine && lpCmdLine[0] != '\0') {
        commandArgs += " ";
        commandArgs += lpCmdLine;
    }

    const std::string commandString = nodeLocation + commandArgs;

    // Use a vector for safer memory management
    std::vector<char> command(commandString.begin(), commandString.end());
    command.push_back('\0');

    if (writeLog) {
        log << "Executing: " << commandString << std::endl;
    }

    // Initialize process structures
    STARTUPINFO si = {};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi = {};

    // Launch the Node.js process
    BOOL success = CreateProcess(
        NULL,                   // No module name (use command line)
        command.data(),         // Command line
        NULL,                   // Process handle not inheritable
        NULL,                   // Thread handle not inheritable
        FALSE,                  // Set handle inheritance to FALSE
        0,                      // No creation flags
        NULL,                   // Use parent's environment block
        NULL,                   // Use parent's starting directory
        &si,                    // Pointer to STARTUPINFO structure
        &pi                     // Pointer to PROCESS_INFORMATION structure
    );

    if (success) {
        if (writeLog) {
            log << "Node.js process created successfully (PID: " << pi.dwProcessId << ")" << std::endl;
        }
        
        // Wait for the Node.js process to exit
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        // Get exit code
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        // Cleanup process handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        if (writeLog) {
            log << "Node.js process exited with code: " << exitCode << std::endl;
        }
        
        if (log.is_open()) {
            log.close();
        }
        
        return static_cast<int>(exitCode);
    } else {
        // Log detailed error information
        DWORD errorCode = GetLastError();
        if (writeLog) {
            log << "ERROR: Failed to create Node.js process" << std::endl;
            log << "Error code: " << errorCode << std::endl;
            
            // Get error message from system
            LPSTR msgBuf = nullptr;
            FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR)&msgBuf,
                0,
                NULL
            );
            if (msgBuf) {
                log << "Error message: " << msgBuf << std::endl;
                LocalFree(msgBuf);
            }
            
            log.close();
        }
        
        return 1;
    }
}

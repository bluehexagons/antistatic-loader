#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    
    struct LocalFreeDeleter {
        void operator()(void* ptr) const {
            if (ptr) LocalFree(ptr);
        }
    };
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
#endif

struct Config {
    std::string logFile;
    std::string nodeLocation;
    std::string gameEntryPoint;
    
    Config() {
#ifdef _WIN32
        logFile = "./debug.txt";
        nodeLocation = "./node.exe";
        gameEntryPoint = "./app/dist/src/engine.js";
#else
        logFile = "./debug.txt";
        nodeLocation = "./node";
        gameEntryPoint = "./app/dist/src/engine.js";
#endif
    }
};

std::string escapeArgument(const std::string& arg) {
#ifdef _WIN32
    if (arg.find_first_of(" \t\n\v\"") != std::string::npos) {
        std::string escaped = "\"";
        for (size_t i = 0; i < arg.length(); ++i) {
            size_t backslashCount = 0;
            while (i < arg.length() && arg[i] == '\\') {
                ++backslashCount;
                ++i;
            }
            
            if (i == arg.length()) {
                escaped.append(backslashCount * 2, '\\');
            } else if (arg[i] == '\"') {
                escaped.append(backslashCount * 2, '\\');
                escaped += "\\\"";
            } else {
                escaped.append(backslashCount, '\\');
                escaped += arg[i];
            }
        }
        escaped += "\"";
        return escaped;
    }
    return arg;
#else
    if (arg.find_first_of(" \t\n\v'\"\\$`!*?[](){};<>|&") != std::string::npos) {
        std::string escaped = "'";
        for (char c : arg) {
            if (c == '\'') {
                escaped += "'\\''";
            } else {
                escaped += c;
            }
        }
        escaped += "'";
        return escaped;
    }
    return arg;
#endif
}

bool fileExists(const std::string& path) {
#ifdef _WIN32
    std::ifstream f(path);
    return f.good();
#else
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
#endif
}

#ifdef _WIN32
// Windows implementation
int runNodeProcess(const std::string& commandString, std::ofstream& log, bool writeLog) {
    std::vector<char> command(commandString.begin(), commandString.end());
    command.push_back('\0');

    if (writeLog) {
        log << "Executing: " << commandString << std::endl;
    }

    STARTUPINFO si = {};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi = {};

    BOOL success = CreateProcess(
        NULL,
        command.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );

    if (success) {
        if (writeLog) {
            log << "Node.js process created (PID: " << pi.dwProcessId << ")" << std::endl;
        }
        
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        if (writeLog) {
            log << "Process exited with code: " << exitCode << std::endl;
        }
        
        return static_cast<int>(exitCode);
    } else {
        DWORD errorCode = GetLastError();
        if (writeLog) {
            log << "ERROR: Failed to create process" << std::endl;
            log << "Error code: " << errorCode << std::endl;
            
            LPSTR errorMessageBuffer = nullptr;
            DWORD msgLen = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&errorMessageBuffer),
                0,
                NULL
            );
            
            std::unique_ptr<void, LocalFreeDeleter> msgBuf(errorMessageBuffer);
            
            if (msgLen > 0 && msgBuf) {
                std::string errorMsg(static_cast<char*>(msgBuf.get()), msgLen);
                size_t end = errorMsg.find_last_not_of(" \n\r\t");
                if (end != std::string::npos) {
                    errorMsg.erase(end + 1);
                    log << "Error message: " << errorMsg << std::endl;
                }
            }
        }
        
        return 1;
    }
}
#else
// Linux/Unix implementation
int runNodeProcess(const std::string& commandString, std::ofstream& log, bool writeLog) {
    if (writeLog) {
        log << "Executing: " << commandString << std::endl;
    }

    pid_t pid = fork();
    
    if (pid < 0) {
        if (writeLog) {
            log << "ERROR: Failed to fork process" << std::endl;
        }
        return 1;
    } else if (pid == 0) {
        // Child process - execute command
        execl("/bin/sh", "sh", "-c", commandString.c_str(), nullptr);
        
        // If execl returns, it failed
        std::cerr << "ERROR: Failed to execute command" << std::endl;
        exit(1);
    } else {
        // Parent process - wait for child
        if (writeLog) {
            log << "Node.js process created (PID: " << pid << ")" << std::endl;
        }
        
        int status;
        waitpid(pid, &status, 0);
        
        int exitCode;
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            // Follow Unix convention: 128 + signal number
            exitCode = 128 + WTERMSIG(status);
        } else {
            exitCode = 1;
        }
        
        if (writeLog) {
            log << "Process exited with code: " << exitCode << std::endl;
        }
        
        return exitCode;
    }
}
#endif

int runLauncher(int argc, char* argv[]) {
    Config config;
    
    std::ofstream log(config.logFile);
    const bool writeLog = log.good();

    if (writeLog) {
        log << "Antistatic Loader Starting..." << std::endl;
        log << "Command line args: " << argc - 1 << std::endl;
    }

    if (!fileExists(config.nodeLocation)) {
        if (writeLog) {
            log << "ERROR: Unable to find Node.js at " << config.nodeLocation << std::endl;
        }
        return 1;
    }

    // Build command: node with security flags, game script, and arguments
    std::string commandString = escapeArgument(config.nodeLocation) + 
                                " --disallow-code-generation-from-strings " + 
                                escapeArgument(config.gameEntryPoint);
    
    // Append command line arguments with proper escaping
    for (int i = 1; i < argc; ++i) {
        commandString += " ";
        commandString += escapeArgument(argv[i]);
    }

    return runNodeProcess(commandString, log, writeLog);
}

#ifdef _WIN32
// Windows entry point for GUI application
int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)nShowCmd;
    
    // Use CommandLineToArgvW for proper argument parsing
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    
    if (argvW == nullptr) {
        return 1;
    }
    
    // Convert to narrow strings
    std::vector<std::string> args;
    std::vector<char*> argv;
    
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        if (size > 0) {
            std::string arg(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &arg[0], size, nullptr, nullptr);
            args.push_back(arg);
        }
    }
    
    LocalFree(argvW);
    
    // Build argv array
    for (auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    
    return runLauncher(static_cast<int>(argv.size()), argv.data());
}
#endif

// Standard entry point for console/Linux
int main(int argc, char* argv[]) {
    return runLauncher(argc, argv);
}

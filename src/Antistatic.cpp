#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
    #include <shellapi.h>
    
    struct LocalFreeDeleter {
        void operator()(void* ptr) const { if (ptr) LocalFree(ptr); }
    };
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
#endif

struct Config {
    std::string logFile = "./debug.txt";
    std::string nodeLocation;
    std::string gameEntryPoint = "./app/dist/src/engine.js";
    
    Config() {
#ifdef _WIN32
        nodeLocation = "./node.exe";
#else
        nodeLocation = "./node";
#endif
    }
};

std::string escapeArgument(const std::string& arg) {
#ifdef _WIN32
    if (arg.find_first_of(" \t\n\v\"") == std::string::npos) return arg;
    
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
    return escaped + "\"";
#else
    if (arg.find_first_of(" \t\n\v'\"\\$`!*?[](){};<>|&") == std::string::npos) return arg;
    
    std::string escaped = "'";
    for (char c : arg)
        escaped += (c == '\'') ? "'\\\''" : std::string(1, c);
    return escaped + "'";
#endif
}

bool fileExists(const std::string& path) {
#ifdef _WIN32
    return std::ifstream(path).good();
#else
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
#endif
}

#ifdef _WIN32
int runNodeProcess(const std::string& cmd, std::ofstream& log, bool writeLog) {
    std::vector<char> command(cmd.begin(), cmd.end());
    command.push_back('\0');

    if (writeLog) log << "Executing: " << cmd << std::endl;

    STARTUPINFO si = {};
    si.cb = sizeof(STARTUPINFO);
    PROCESS_INFORMATION pi = {};

    if (!CreateProcess(NULL, command.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        if (writeLog) {
            log << "ERROR: Process creation failed (code " << GetLastError() << ")" << std::endl;
            LPSTR errorMsg = nullptr;
            DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, GetLastError(), 0, reinterpret_cast<LPSTR>(&errorMsg), 0, NULL);
            std::unique_ptr<void, LocalFreeDeleter> msgBuf(errorMsg);
            if (msgLen > 0 && msgBuf) {
                std::string msg(static_cast<char*>(msgBuf.get()), msgLen);
                size_t end = msg.find_last_not_of(" \n\r\t");
                if (end != std::string::npos) msg.erase(end + 1);
                log << msg << std::endl;
            }
        }
        return 1;
    }

    if (writeLog) log << "Process started (PID " << pi.dwProcessId << ")" << std::endl;
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    
    if (writeLog) log << "Exit code: " << exitCode << std::endl;
    return static_cast<int>(exitCode);
}
#else
int runNodeProcess(const std::string& cmd, std::ofstream& log, bool writeLog) {
    if (writeLog) log << "Executing: " << cmd << std::endl;

    pid_t pid = fork();
    if (pid < 0) {
        if (writeLog) log << "ERROR: fork failed" << std::endl;
        return 1;
    }
    
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
        std::cerr << "ERROR: exec failed" << std::endl;
        exit(1);
    }

    if (writeLog) log << "Process started (PID " << pid << ")" << std::endl;
    
    int status;
    waitpid(pid, &status, 0);
    
    int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 
                   WIFSIGNALED(status) ? 128 + WTERMSIG(status) : 1;
    
    if (writeLog) log << "Exit code: " << exitCode << std::endl;
    return exitCode;
}
#endif

int runLauncher(int argc, char* argv[]) {
    Config config;
    std::ofstream log(config.logFile);
    bool writeLog = log.good();

    if (writeLog) {
        log << "Antistatic Loader" << std::endl;
        log << "Args: " << argc - 1 << std::endl;
    }

    if (!fileExists(config.nodeLocation)) {
        if (writeLog) log << "ERROR: Node.js not found at " << config.nodeLocation << std::endl;
        return 1;
    }

    std::string cmd = escapeArgument(config.nodeLocation) + 
                      " --disallow-code-generation-from-strings " + 
                      escapeArgument(config.gameEntryPoint);
    
    for (int i = 1; i < argc; ++i) {
        cmd += " " + escapeArgument(argv[i]);
    }

    return runNodeProcess(cmd, log, writeLog);
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW) return 1;
    
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        int size = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
        if (size > 0) {
            std::string arg(size - 1, '\0');
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, &arg[0], size, nullptr, nullptr);
            args.push_back(arg);
        }
    }
    LocalFree(argvW);
    
    std::vector<char*> argv;
    for (auto& arg : args) argv.push_back(const_cast<char*>(arg.c_str()));
    
    return runLauncher(static_cast<int>(argv.size()), argv.data());
}
#endif

int main(int argc, char* argv[]) {
    return runLauncher(argc, argv);
}


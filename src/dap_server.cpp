#include "dap_server.h"

#include <iostream>
#include <sstream>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#endif

namespace uzpp {

DapServer::DapServer() {}

DapServer::~DapServer() {
    if (gdbRunning_) {
        stopGdbThread_ = true;
        sendGdbCommand("-gdb-exit");
        if (gdbThread_.joinable()) gdbThread_.join();
#ifdef _WIN32
        if (gdbProcess_) TerminateProcess((HANDLE)gdbProcess_, 0);
        if (gdbIn_) CloseHandle((HANDLE)gdbIn_);
        if (gdbOut_) CloseHandle((HANDLE)gdbOut_);
#else
        if (gdbPid_ > 0) kill(gdbPid_, SIGKILL);
        if (gdbIn_ != -1) close(gdbIn_);
        if (gdbOut_ != -1) close(gdbOut_);
#endif
    }
}

void DapServer::run() {
    while (std::cin) {
        std::string line;
        int contentLength = 0;
        
        while (std::getline(std::cin, line)) {
            if (line.empty() || line == "\r") break;
            if (line.find("Content-Length: ") == 0) {
                contentLength = std::stoi(line.substr(16));
            }
        }
        
        if (contentLength > 0) {
            std::string content(contentLength, ' ');
            std::cin.read(&content[0], contentLength);
            handleMessage(content);
        }
    }
}

void DapServer::sendMessage(const std::string& jsonContent) {
    std::lock_guard<std::mutex> lock(sendMutex_);
    std::cout << "Content-Length: " << jsonContent.size() << "\r\n\r\n" << jsonContent;
    std::cout.flush();
}

std::string DapServer::extractJsonString(const std::string& json, const std::string& key) {
    std::string target = "\"" + key + "\":";
    size_t pos = json.find(target);
    if (pos == std::string::npos) return "";
    pos += target.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '"')) pos++;
    size_t end = pos;
    while (end < json.length() && json[end] != '"') end++;
    return json.substr(pos, end - pos);
}

int DapServer::extractSeq(const std::string& json) {
    size_t pos = json.find("\"seq\":");
    if (pos == std::string::npos) return 0;
    pos += 6;
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    size_t end = pos;
    while (end < json.length() && isdigit(json[end])) end++;
    if (pos == end) return 0;
    return std::stoi(json.substr(pos, end - pos));
}

int DapServer::extractInt(const std::string& json, const std::string& key) {
    std::string target = "\"" + key + "\":";
    size_t pos = json.find(target);
    if (pos == std::string::npos) return 0;
    pos += target.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    size_t end = pos;
    while (end < json.length() && (isdigit(json[end]) || json[end] == '-')) end++;
    if (pos == end) return 0;
    return std::stoi(json.substr(pos, end - pos));
}

void DapServer::launchGdb(const std::string& program) {
#ifdef _WIN32
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;

    CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0);
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
    CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0);
    SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0);

    gdbIn_ = hChildStd_IN_Wr;
    gdbOut_ = hChildStd_OUT_Rd;

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOA siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    std::string cmdLine = "gdb --interpreter=mi3 \"" + program + "\"";
    std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
    cmdBuf.push_back(0);

    CreateProcessA(NULL, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, NULL, &siStartInfo, &piProcInfo);
    gdbProcess_ = piProcInfo.hProcess;
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(hChildStd_IN_Rd);
#else
    int pipeIn[2], pipeOut[2];
    if (pipe(pipeIn) != 0 || pipe(pipeOut) != 0) return;
    
    pid_t pid = fork();
    if (pid == 0) {
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        dup2(pipeOut[1], STDERR_FILENO);
        close(pipeIn[0]); close(pipeIn[1]);
        close(pipeOut[0]); close(pipeOut[1]);
        execlp("gdb", "gdb", "--interpreter=mi3", program.c_str(), nullptr);
        exit(1);
    } else {
        gdbPid_ = pid;
        gdbIn_ = pipeIn[1];
        gdbOut_ = pipeOut[0];
        close(pipeIn[0]);
        close(pipeOut[1]);
    }
#endif

    gdbRunning_ = true;
    gdbThread_ = std::thread(&DapServer::gdbReadLoop, this);
}

void DapServer::sendGdbCommand(const std::string& cmd) {
    if (!gdbRunning_) return;
    std::string fullCmd = cmd + "\n";
#ifdef _WIN32
    DWORD written;
    WriteFile((HANDLE)gdbIn_, fullCmd.c_str(), fullCmd.size(), &written, NULL);
#else
    write(gdbIn_, fullCmd.c_str(), fullCmd.size());
#endif
}

void DapServer::gdbReadLoop() {
    char buffer[4096];
    std::string lineBuffer;
    while (!stopGdbThread_ && gdbRunning_) {
#ifdef _WIN32
        DWORD bytesRead;
        if (!ReadFile((HANDLE)gdbOut_, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0) break;
#else
        int bytesRead = read(gdbOut_, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) break;
#endif
        buffer[bytesRead] = '\0';
        lineBuffer += buffer;
        size_t pos;
        while ((pos = lineBuffer.find('\n')) != std::string::npos) {
            std::string line = lineBuffer.substr(0, pos);
            lineBuffer.erase(0, pos + 1);
            processGdbOutput(line);
        }
    }
}

void DapServer::processGdbOutput(const std::string& output) {
    if (output.empty()) return;
    
    if (output.starts_with("*stopped")) {
        std::string event = "{\"type\":\"event\",\"event\":\"stopped\",\"body\":{\"reason\":\"step\",\"threadId\":1}}";
        sendMessage(event);
    } else if (output.starts_with("~\"")) {
        // Console output stream translation
        std::string msg = output.substr(2, output.length() - 3);
        std::string event = "{\"type\":\"event\",\"event\":\"output\",\"body\":{\"category\":\"console\",\"output\":\"" + msg + "\\n\"}}";
        sendMessage(event);
    } else if (output.starts_with("^done") || output.starts_with("^error")) {
        std::lock_guard<std::mutex> lock(gdbResponseMutex_);
        lastGdbResponse_ = output;
        gdbResponseReady_ = true;
        gdbResponseCv_.notify_one();
    }
}

void DapServer::handleMessage(const std::string& content) {
    std::string type = extractJsonString(content, "type");
    if (type == "request") {
        std::string command = extractJsonString(content, "command");
        int seq = extractSeq(content);
        
        if (command == "initialize") {
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"initialize\",\"body\":{\"supportsConfigurationDoneRequest\":true,\"supportsEvaluateForHovers\":true,\"supportsStepBack\":false}}";
            sendMessage(response);
        } else if (command == "launch" || command == "attach") {
            std::string program = extractJsonString(content, "program");
            if (!program.empty()) {
                launchGdb(program);
            }
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"" + command + "\"}";
            sendMessage(response);
            sendMessage("{\"seq\":" + std::to_string(seq + 2) + ",\"type\":\"event\",\"event\":\"initialized\"}");
        } else if (command == "setBreakpoints") {
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"setBreakpoints\",\"body\":{\"breakpoints\":[]}}";
            sendMessage(response);
        } else if (command == "configurationDone") {
            sendGdbCommand("-exec-run");
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"configurationDone\"}";
            sendMessage(response);
        } else if (command == "next") {
            sendGdbCommand("-exec-next");
            sendMessage("{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"next\"}");
        } else if (command == "stepIn") {
            sendGdbCommand("-exec-step");
            sendMessage("{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"stepIn\"}");
        } else if (command == "stepOut") {
            sendGdbCommand("-exec-finish");
            sendMessage("{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"stepOut\"}");
        } else if (command == "continue") {
            sendGdbCommand("-exec-continue");
            sendMessage("{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"continue\"}");
        } else if (command == "pause") {
            sendGdbCommand("-exec-interrupt");
            sendMessage("{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"pause\"}");
        } else if (command == "threads") {
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"threads\",\"body\":{\"threads\":[{\"id\":1,\"name\":\"Asosiy oqim (Main Thread)\"}]}}";
            sendMessage(response);
        } else if (command == "stackTrace") {
            sendGdbCommand("-stack-list-frames");
            std::string mi = waitForGdbResponse();
            std::string frames = parseGdbFrames(mi);
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"stackTrace\",\"body\":{\"stackFrames\":" + frames + "}}";
            sendMessage(response);
        } else if (command == "scopes") {
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"scopes\",\"body\":{\"scopes\":[{\"name\":\"Mahalliy (Local)\",\"variablesReference\":1,\"expensive\":false}]}}";
            sendMessage(response);
        } else if (command == "variables") {
            sendGdbCommand("-stack-list-variables --all-values");
            std::string mi = waitForGdbResponse();
            std::string vars = parseGdbVariables(mi);
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"variables\",\"body\":{\"variables\":" + vars + "}}";
            sendMessage(response);
        } else if (command == "evaluate") {
            std::string expr = extractJsonString(content, "expression");
            sendGdbCommand("-data-evaluate-expression \"" + expr + "\"");
            std::string mi = waitForGdbResponse();
            std::string result = parseGdbEvaluation(mi);
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"evaluate\",\"body\":{\"result\":\"" + result + "\",\"variablesReference\":0}}";
            sendMessage(response);
        } else if (command == "disconnect") {
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"disconnect\"}";
            sendMessage(response);
            std::exit(0);
        } else {
            // Acknowledge unknown commands
            std::string response = "{\"seq\":" + std::to_string(seq + 1) + ",\"type\":\"response\",\"request_seq\":" + std::to_string(seq) + ",\"success\":true,\"command\":\"" + command + "\"}";
            sendMessage(response);
        }
    }
}

// Implementation of missing DAP methods
std::string DapServer::waitForGdbResponse() {
    std::unique_lock<std::mutex> lock(gdbResponseMutex_);
    gdbResponseCv_.wait(lock, [this]{ return gdbResponseReady_; });
    gdbResponseReady_ = false;
    return lastGdbResponse_;
}

std::string DapServer::parseGdbFrames(const std::string& gdbOutput) {
    if (gdbOutput.empty() || gdbOutput.find("^error") != std::string::npos) return "[]";
    
    std::string json = "[";
    size_t pos = 0;
    bool first = true;
    while ((pos = gdbOutput.find("frame={", pos)) != std::string::npos) {
        if (!first) json += ",";
        first = false;
        
        auto extractStr = [&](const std::string& key) {
            size_t kPos = gdbOutput.find(key + "=\"", pos);
            if (kPos == std::string::npos) return std::string("");
            size_t endScope = gdbOutput.find("}", pos);
            if (kPos > endScope) return std::string("");
            kPos += key.length() + 2;
            size_t end = gdbOutput.find("\"", kPos);
            return gdbOutput.substr(kPos, end - kPos);
        };
        
        std::string level = extractStr("level");
        std::string func = extractStr("func");
        std::string file = extractStr("file");
        std::string fullname = extractStr("fullname");
        std::string lineStr = extractStr("line");
        
        // Windows/MinGW paths in GDB can have double backslashes
        std::string escapedPath;
        for (char c : fullname) {
            if (c == '\\') escapedPath += "\\\\";
            else escapedPath += c;
        }
        
        int line = lineStr.empty() ? 0 : std::stoi(lineStr);
        int id = level.empty() ? 0 : std::stoi(level);
        
        json += "{\"id\":" + std::to_string(id) + ",\"name\":\"" + func + "\",\"line\":" + std::to_string(line) + ",\"column\":0,\"source\":{\"name\":\"" + file + "\",\"path\":\"" + escapedPath + "\"}}";
        
        pos += 7;
    }
    json += "]";
    return json;
}

std::string DapServer::parseGdbVariables(const std::string& gdbOutput) {
    if (gdbOutput.empty() || gdbOutput.find("^error") != std::string::npos) return "[]";
    
    std::string json = "[";
    size_t pos = 0;
    bool first = true;
    while ((pos = gdbOutput.find("{name=\"", pos)) != std::string::npos) {
        if (!first) json += ",";
        first = false;
        
        auto extractStr = [&](const std::string& key) {
            size_t kPos = gdbOutput.find(key + "=\"", pos);
            if (kPos == std::string::npos) return std::string("");
            size_t endScope = gdbOutput.find("}", pos);
            if (kPos > endScope) return std::string("");
            kPos += key.length() + 2;
            size_t end = gdbOutput.find("\"", kPos);
            return gdbOutput.substr(kPos, end - kPos);
        };
        
        std::string name = extractStr("name");
        std::string type = extractStr("type");
        std::string value = extractStr("value");
        if (value.empty()) value = "{...}";
        
        std::string escapedValue;
        for (char c : value) {
            if (c == '"') escapedValue += "\\\"";
            else if (c == '\\') escapedValue += "\\\\";
            else escapedValue += c;
        }
        
        json += "{\"name\":\"" + name + "\",\"value\":\"" + escapedValue + "\",\"type\":\"" + type + "\",\"variablesReference\":0}";
        
        pos += 6;
    }
    json += "]";
    return json;
}

std::string DapServer::parseGdbEvaluation(const std::string& gdbOutput) {
    if (gdbOutput.empty() || gdbOutput.find("^error") != std::string::npos) return "null";
    
    size_t pos = gdbOutput.find("value=\"");
    if (pos != std::string::npos) {
        pos += 7;
        size_t end = gdbOutput.find("\"", pos);
        std::string value = gdbOutput.substr(pos, end - pos);
        std::string escaped;
        for (char c : value) {
            if (c == '"') escaped += "\\\"";
            else if (c == '\\') escaped += "\\\\";
            else escaped += c;
        }
        return escaped;
    }
    return "null";
}

} // namespace uzpp
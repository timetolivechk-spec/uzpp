#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <condition_variable>

namespace uzpp {

class DapServer {
public:
    DapServer();
    ~DapServer();
    void run();

private:
    void handleMessage(const std::string& content);
    void sendMessage(const std::string& jsonContent);
    std::string extractJsonString(const std::string& json, const std::string& key);
    int extractSeq(const std::string& json);
    int extractInt(const std::string& json, const std::string& key);

    // GDB Integratsiyasi
    void launchGdb(const std::string& program);
    void sendGdbCommand(const std::string& cmd);
    void gdbReadLoop();
    void processGdbOutput(const std::string& output);
    std::string waitForGdbResponse();
    std::string parseGdbFrames(const std::string& mi);
    std::string parseGdbVariables(const std::string& mi);
    std::string parseGdbEvaluation(const std::string& mi);

    bool gdbRunning_ = false;
    std::thread gdbThread_;
    std::atomic<bool> stopGdbThread_{false};
    std::mutex sendMutex_;

#ifdef _WIN32
    void* gdbProcess_ = nullptr;
    void* gdbIn_ = nullptr;
    void* gdbOut_ = nullptr;
#else
    int gdbPid_ = -1;
    int gdbIn_ = -1;
    int gdbOut_ = -1;
#endif

    std::mutex gdbResponseMutex_;
    std::condition_variable gdbResponseCv_;
    std::string lastGdbResponse_;
    bool gdbResponseReady_ = false;
};

} // namespace uzpp
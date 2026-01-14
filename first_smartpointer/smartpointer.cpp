#include <iostream>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

class LogFileManager {
public:
    LogFileManager() = default;
    ~LogFileManager() = default;

    LogFileManager(const LogFileManager&) = delete;
    LogFileManager& operator=(const LogFileManager&) = delete;

    LogFileManager(LogFileManager&&) noexcept = default;
    LogFileManager& operator=(LogFileManager&&) noexcept = default;

    void openLogFile(const std::string& filename) {
        if (filename.empty()) {
            throw std::invalid_argument("openLogFile: filename is empty");
        }

        if (files_.find(filename) != files_.end()) {
            return;
        }

        auto fs = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!(*fs)) {
            throw std::runtime_error("openLogFile: failed to open file: " + filename);
        }

        files_.emplace(filename, std::move(fs));
    }

    void writeLog(const std::string& filename, const std::string& message) {
        if (filename.empty()) {
            throw std::invalid_argument("writeLog: filename is empty");
        }

        auto it = files_.find(filename);
        if (it == files_.end() || !it->second) {
            throw std::runtime_error("writeLog: file is not opened: " + filename);
        }

        std::ofstream& out = *(it->second);
        if (!out) {
            throw std::runtime_error("writeLog: stream is not valid: " + filename);
        }

        out << "[" << currentTimestamp() << "] " << message << "\n";

        if (!out) {
            throw std::runtime_error("writeLog: failed to write: " + filename);
        }

        out.flush();
        if (!out) {
            throw std::runtime_error("writeLog: failed to flush: " + filename);
        }
    }

    std::vector<std::string> readLogs(const std::string& filename) const {
        if (filename.empty()) {
            throw std::invalid_argument("readLogs: filename is empty");
        }

        std::ifstream in(filename);
        if (!in) {
            throw std::runtime_error("readLogs: failed to open file for reading: " + filename);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) {
            lines.push_back(line);
        }

        if (in.bad()) {
            throw std::runtime_error("readLogs: failed while reading: " + filename);
        }

        return lines;
    }

    void closeLogFile(const std::string& filename) {
        auto it = files_.find(filename);
        if (it == files_.end()) {
            return; 
        }

        if (it->second) {
            it->second->close();
        }
        files_.erase(it);
    }

private:
    static std::string currentTimestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t t = system_clock::to_time_t(now);

        std::tm tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &t);
#else
        localtime_r(&t, &tm_buf);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<std::ofstream>> files_;
};

int main() {
    try {
        LogFileManager manager;

        manager.openLogFile("error.log");
        manager.openLogFile("debug.log");
        manager.openLogFile("info.log");

        manager.writeLog("error.log", "Database connection failed");
        manager.writeLog("debug.log", "User login attempt");
        manager.writeLog("info.log", "Server started successfully");

        std::vector<std::string> errorLogs = manager.readLogs("error.log");

        std::cout << "// error.log 파일 내용\n";
        for (const auto& line : manager.readLogs("error.log")) {
            std::cout << line << "\n";
        }

        std::cout << "\n// debug.log 파일 내용\n";
        for (const auto& line : manager.readLogs("debug.log")) {
            std::cout << line << "\n";
        }

        std::cout << "\n// info.log 파일 내용\n";
        for (const auto& line : manager.readLogs("info.log")) {
            std::cout << line << "\n";
        }

        std::cout << "\n// readLogs 반환값\n";
        if (!errorLogs.empty()) {
            std::cout << "errorLogs[0] = \"" << errorLogs[0] << "\"\n";
        } else {
            std::cout << "errorLogs is empty\n";
        }

        manager.closeLogFile("error.log");
        manager.closeLogFile("debug.log");
        manager.closeLogFile("info.log");

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}

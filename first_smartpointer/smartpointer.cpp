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
#include <filesystem>

class LogFileManager {
public:
    LogFileManager() = default;
    ~LogFileManager() = default;

    LogFileManager(const LogFileManager&) = delete;
    LogFileManager& operator=(const LogFileManager&) = delete;

    LogFileManager(LogFileManager&&) noexcept = default;
    LogFileManager& operator=(LogFileManager&&) noexcept = default;

    void openLogFile(const std::string& filename) {
        if (filename.empty()) throw std::runtime_error("filename empty");

        if (files_.count(filename)) return;

        auto out = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!*out) throw std::runtime_error("open failed: " + filename);

        files_[filename] = std::move(out);
    }

    void writeLog(const std::string& filename, const std::string& message) {
        if (!files_.count(filename)) throw std::runtime_error("not opened: " + filename);

        *files_[filename] << "[" << timestamp() << "] " << message << "\n";
        files_[filename]->flush();
    }

    std::vector<std::string> readLogs(const std::string& filename) const {
        std::ifstream in(filename);
        if (!in) throw std::runtime_error("read open failed: " + filename);

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(in, line)) lines.push_back(line);
        return lines;
    }

    void closeLogFile(const std::string& filename) {
        auto it = files_.find(filename);
        if (it == files_.end()) return;
        it->second->close();
        files_.erase(it); 
    }

private:
    static std::string timestamp() {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t t = system_clock::to_time_t(now);

        std::tm tm_buf{};
        localtime_s(&tm_buf, &t);

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
        std::cout << "cwd = " << std::filesystem::current_path() << "\n";

        std::cout << "// error.log 파일 내용\n";
        for (const auto& line : manager.readLogs("error.log")) std::cout << line << "\n";

        std::cout << "\n// debug.log 파일 내용\n";
        for (const auto& line : manager.readLogs("debug.log")) std::cout << line << "\n";

        std::cout << "\n// info.log 파일 내용\n";
        for (const auto& line : manager.readLogs("info.log")) std::cout << line << "\n";

        manager.closeLogFile("error.log");
        manager.closeLogFile("debug.log");
        manager.closeLogFile("info.log");
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}

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

    // 복사 금지 (파일 핸들(자원) 중복 소유 방지)
    LogFileManager(const LogFileManager&) = delete;
    LogFileManager& operator=(const LogFileManager&) = delete;

    // 이동 허용
    LogFileManager(LogFileManager&&) noexcept = default;
    LogFileManager& operator=(LogFileManager&&) noexcept = default;

    // filename 로그 파일을 열어(없으면 생성) 핸들을 관리합니다.
    void openLogFile(const std::string& filename) {
        if (filename.empty()) {
            throw std::invalid_argument("openLogFile: filename is empty");
        }

        // 이미 열려 있으면 그대로 둡니다.
        if (files_.find(filename) != files_.end()) {
            return;
        }

        // 파일을 append 모드로 열어 로그를 계속 이어쓰기
        auto fs = std::make_unique<std::ofstream>(filename, std::ios::app);
        if (!(*fs)) {
            throw std::runtime_error("openLogFile: failed to open file: " + filename);
        }

        files_.emplace(filename, std::move(fs));
    }

    // filename에 [timestamp] message 형태로 기록합니다.
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

        // 쓰기 실패 체크
        if (!out) {
            throw std::runtime_error("writeLog: failed to write: " + filename);
        }

        // 제출/테스트에서 즉시 반영을 원할 때 flush가 안전합니다.
        out.flush();
        if (!out) {
            throw std::runtime_error("writeLog: failed to flush: " + filename);
        }
    }

    // filename의 전체 로그를 한 줄씩 vector<string>으로 반환합니다.
    // 읽기는 별도의 ifstream을 열어 읽습니다(기존 ofstream과 포지션 충돌 방지).
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

    // filename 파일 핸들을 닫습니다(스마트 포인터 reset/erase로 자동 정리).
    void closeLogFile(const std::string& filename) {
        auto it = files_.find(filename);
        if (it == files_.end()) {
            return; // 닫을 게 없으면 조용히 무시
        }

        if (it->second) {
            // ofstream은 소멸 시 close되지만, 명시적으로 close해도 무방합니다.
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
    // 여러 파일을 동시에 관리: filename -> 스마트 포인터(ofstream)
    std::unordered_map<std::string, std::unique_ptr<std::ofstream>> files_;
};

// 예시 실행(이미지의 입력/출력 흐름을 재현하는 샘플)
int main() {
    try {
        LogFileManager manager;

        // 입력값(왼쪽 예시)과 동일한 흐름
        manager.openLogFile("error.log");
        manager.openLogFile("debug.log");
        manager.openLogFile("info.log");

        manager.writeLog("error.log", "Database connection failed");
        manager.writeLog("debug.log", "User login attempt");
        manager.writeLog("info.log", "Server started successfully");

        std::vector<std::string> errorLogs = manager.readLogs("error.log");

        // 출력 예시(오른쪽 형태에 맞춰 보여주기)
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

        // 정리(명시적으로 닫기)
        manager.closeLogFile("error.log");
        manager.closeLogFile("debug.log");
        manager.closeLogFile("info.log");

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
}

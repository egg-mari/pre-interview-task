#include <iostream>
#include <vector>
#include <thread>
#include <string>
#include <chrono>
#include <algorithm>

class ParallelProcessor {
public:
    ParallelProcessor(int threads) : threads_(threads) {
        if (threads_ <= 0) threads_ = 1;
    }

    int thread_count() const { return threads_; }

    template<typename T, typename R, typename Func>
    std::vector<R> parallel_map(const std::vector<T>& input, Func func) const {
        std::vector<R> output(input.size());

        int threads = threads_;
        if ((int)input.size() < threads) threads = (int)input.size();
        if (threads <= 0) threads = 1;

        std::vector<std::thread> workers;
        workers.reserve(threads);

        auto work = [&](int tid) {
            std::size_t n = input.size();
            std::size_t chunk = n / threads;
            std::size_t start = (std::size_t)tid * chunk;
            std::size_t end = (tid == threads - 1) ? n : start + chunk;

            for (std::size_t i = start; i < end; ++i) {
                output[i] = func(input[i]);
            }
        };

        for (int i = 0; i < threads; ++i) workers.emplace_back(work, i);
        for (auto& th : workers) th.join();

        return output;
    }

private:
    int threads_;
};

static int clamp255(int x) {
    if (x < 0) return 0;
    if (x > 255) return 255;
    return x;
}

int main() {
    std::size_t N = 1'000'000;
    std::vector<int> pixelData(N);
    for (std::size_t i = 0; i < N; ++i) pixelData[i] = (int)i;

    ParallelProcessor processor(4);

    auto brightenedImage = processor.parallel_map<int, int>(pixelData, [](int pixel) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        return clamp255(pixel + 50);
    });

    auto pixelStrings = processor.parallel_map<int, std::string>(pixelData, [](int pixel) {
        return std::string("pixel_") + std::to_string(pixel);
    });

    auto squaredPixels = processor.parallel_map<int, int>(pixelData, [](int pixel) {
        return pixel * pixel;
    });

    std::cout << "// brightenedImage 결과\n";
    std::cout << "brightenedImage[0] = " << brightenedImage[0] << "\n";
    std::cout << "brightenedImage[1] = " << brightenedImage[1] << "\n";
    std::cout << "brightenedImage[100] = " << brightenedImage[100] << "\n";
    std::cout << "brightenedImage[999999] = " << brightenedImage[999999] << "\n\n";

    std::cout << "// pixelStrings 결과\n";
    std::cout << "pixelStrings[0] = \"" << pixelStrings[0] << "\"\n";
    std::cout << "pixelStrings[1] = \"" << pixelStrings[1] << "\"\n";
    std::cout << "pixelStrings[100] = \"" << pixelStrings[100] << "\"\n\n";

    std::cout << "// squaredPixels 결과\n";
    std::cout << "squaredPixels[0] = " << squaredPixels[0] << "\n";
    std::cout << "squaredPixels[1] = " << squaredPixels[1] << "\n";
    std::cout << "squaredPixels[10] = " << squaredPixels[10] << "\n\n";

    // 성능 측정(헬퍼 없이 대충)
    std::vector<int> seq_out(pixelData.size());

    auto t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < pixelData.size(); ++i) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        seq_out[i] = clamp255(pixelData[i] + 50);
    }
    auto t1 = std::chrono::steady_clock::now();
    auto seq_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    auto t2 = std::chrono::steady_clock::now();
    auto par_out = processor.parallel_map<int, int>(pixelData, [](int pixel) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
        return clamp255(pixel + 50);
    });
    auto t3 = std::chrono::steady_clock::now();
    auto par_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();

    std::cout << "// 성능 측정 결과 출력\n";
    std::cout << "Processing " << pixelData.size() << " elements with " << processor.thread_count() << " threads\n";
    std::cout << "Sequential time: ~" << seq_ms << "ms\n";
    std::cout << "Parallel time: ~" << par_ms << "ms\n";
    if (par_ms > 0) {
        std::cout << "Speedup: ~" << (double)seq_ms / (double)par_ms << "x\n";
    } else {
        std::cout << "Speedup: (parallel time too small)\n";
    }
}

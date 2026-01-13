#include <iostream>
#include "CircularBuffer.h"

int main() {
    CircularBuffer<double> tempBuffer(5);

    std::cout << "tempBuffer.size() = " << tempBuffer.size() << "\n";
    std::cout << "tempBuffer.capacity() = " << tempBuffer.capacity() << "\n";
    std::cout << "tempBuffer.empty() = " << (tempBuffer.empty() ? "true" : "false") << "\n";
    return 0;
}

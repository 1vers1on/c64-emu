// #define TEST_6502

#ifdef TEST_6502
#include <bus.hpp>
#include <chrono>
#include <cpu.hpp>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    Bus bus;
    CPU cpu(&bus);

    std::ifstream file("6502test.bin", std::ios::binary | std::ios::in);
    if(!file.is_open()) {
        std::cerr << "Failed to open file: 6502test.bin\n";
        return 1;
    }

    std::vector<uint8_t> data = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});

    bus.writeBytes(0x0000, data.data(), data.size());

    cpu.powerOn();
    cpu.reset();

    while(true) {
        cpu.executeOnce();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
#endif
#ifdef __EMSCRIPTEN__
#include <sys/types.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <System.hpp>
#include <algorithm>
#include <cctype>
#include <array>
#include <thread>
#include <chrono>
#include <emscripten.h>

std::array<uint32_t, 40 * 25 * 8 * 8> lastFramebuffer = {};
std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();

System emulatorSystem;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void keyDown(const char* key) {
        std::string keyStr(key);
        emulatorSystem.input->setKeyPressed(keyStr, true);
    }

    EMSCRIPTEN_KEEPALIVE
    int getClockSpeed() {
        return emulatorSystem.clockSpeed;
    }

    EMSCRIPTEN_KEEPALIVE
    void keyUp(const char* key) {
        std::string keyStr(key);
        emulatorSystem.input->setKeyPressed(keyStr, false);
    }

    EMSCRIPTEN_KEEPALIVE
    void writeToMemory(uint16_t addr, uint8_t data) {
        std::cout << "Writing to address: " << std::hex << addr << " with data: " << static_cast<int>(data) << std::dec << std::endl;
        emulatorSystem.bus->write(addr, data);
    }

    EMSCRIPTEN_KEEPALIVE
    uint8_t readFromMemory(uint16_t addr) {
        std::cout << "Reading from address: " << std::hex << addr << std::dec << std::endl;
        return emulatorSystem.bus->read(addr);
    }
    
    EMSCRIPTEN_KEEPALIVE
    uint32_t* getFramebuffer() {
        return lastFramebuffer.data();
    }

    EMSCRIPTEN_KEEPALIVE
    void startEmulator() {
        std::cout << "Starting emulator" << std::endl;
        emulatorSystem.vic->setFramebufferCallback([](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
            std::memcpy(lastFramebuffer.data(), screen.data(), screen.size() * sizeof(uint32_t));
        });
        emulatorSystem.powerOn();

        while (true) {
            emulatorSystem.step();

            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = currentTime - lastTime;

            if (elapsed.count() >= 0.5) {
                lastTime = currentTime;

                EM_ASM({
                    processOtherStuff();
                });

                emulatorSystem.vic->needsRender = true;
            }

            if (emulatorSystem.vic->needsRender) {
                emscripten_sleep(0);
                EM_ASM({
                    framebufferReady();
                });
                emulatorSystem.vic->needsRender = false;
            }
        }
    }
} // extern "C"
#endif

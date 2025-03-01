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


auto lastUpdate = std::chrono::high_resolution_clock::now();

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    uint32_t* getFramebuffer() {
        return lastFramebuffer.data();
    }

    void continueEmulation() {
        
    }

    EMSCRIPTEN_KEEPALIVE
    void startEmulator() {
        std::cout << "Starting emulator" << std::endl;
        System system;
        system.vic->setFramebufferCallback([](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
            std::memcpy(lastFramebuffer.data(), screen.data(), screen.size() * sizeof(uint32_t));
        });
        system.powerOn();

        while (true) {
            system.step();

            auto now = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = now - lastUpdate;

            if (elapsed.count() >= 0.1) {
                EM_ASM({
                    processOtherStuff();
                });
                lastUpdate = now;
            }

            if (system.vic->needsRender) {
                EM_ASM({
                    framebufferReady();
                });
                system.vic->needsRender = false;
            }
        }
    }
} // extern "C"
#endif
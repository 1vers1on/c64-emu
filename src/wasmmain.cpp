#ifdef __EMSCRIPTEN__
#include <sys/types.h>
#include <vector>
#include <cstdint>
#include <cstring>
#include <system.hpp>
#include <algorithm>
#include <cctype>
#include <array>
#include <thread>
#include <chrono>
#include <emscripten.h>

std::array<uint32_t, 40 * 25 * 8 * 8> lastFramebuffer = {};
// std::array<uint32_t, 40 * 25 * 8 * 8> lastLastFramebuffer = {};
std::vector<uint32_t> fbDiff = {};
std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();

System emulatorSystem;
bool paused = false;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void reset() {
        std::cout << "Resetting emulator" << std::endl;
        emulatorSystem.reset();
    }

    void pause() {
        paused = true;
    }

    void resume() {
        paused = false;
    }

    uint8_t* getMemory() {
        return emulatorSystem.bus->ram;
    }

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
    uint32_t getDiffSize() {
        return fbDiff.size();
    }

    EMSCRIPTEN_KEEPALIVE
    uint32_t* getDiff() {
        return fbDiff.data();
    }

    EMSCRIPTEN_KEEPALIVE
    void startEmulator() {
        std::cout << "Starting emulator" << std::endl;
        emulatorSystem.vic->setFramebufferCallback([](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
            fbDiff.clear();
            
            size_t i = 0;
            while (i < screen.size()) {
                if (screen[i] != lastFramebuffer[i]) {
                    size_t runStart = i;
                    uint32_t color = screen[i];
                    
                    do {
                        i++;
                    } while (i < screen.size() && 
                            screen[i] == color && 
                            (screen[i] != lastFramebuffer[i] || i == runStart + 1));
                    
                    fbDiff.push_back(runStart);
                    fbDiff.push_back(i - runStart);
                    fbDiff.push_back(color);
                } else {
                    i++;
                }
            }
            
            
            std::memcpy(lastFramebuffer.data(), screen.data(), screen.size() * sizeof(uint32_t));
        });
        emulatorSystem.powerOn();

        while (true) {
            emulatorSystem.step();
            if (paused) {
                emscripten_sleep(5);
                continue;
            }

            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = currentTime - lastTime;

            if (elapsed.count() >= 0.5) {
                lastTime = currentTime;

                EM_ASM({
                    processOtherStuff();
                });
                // std::cout << getClockSpeed() << std::endl;
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

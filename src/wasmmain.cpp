#if defined(__EMSCRIPTEN__)
#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <iostream>
#include <sys/types.h>
#include <system.hpp>
#include <thread>
#include <vector>

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
    std::cout << "Writing to address: " << std::hex << addr
              << " with data: " << static_cast<int>(data) << std::dec << std::endl;
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
SidState getSidState() {
    SidState state = emulatorSystem.sid->getState();
    return state;
}

EMSCRIPTEN_BINDINGS(sid_state) {
    emscripten::value_object<SidState>("SidState")
        .field("v1Frequency", &SidState::v1Frequency)
        .field("v1PulseWidth", &SidState::v1PulseWidth)
        .field("v1On", &SidState::v1On)
        .field("v1Sync", &SidState::v1Sync)
        .field("v1RingMod", &SidState::v1RingMod)
        .field("v1Disable", &SidState::v1Disable)
        .field("v1Triangle", &SidState::v1Triangle)
        .field("v1Sawtooth", &SidState::v1Sawtooth)
        .field("v1Pulse", &SidState::v1Pulse)
        .field("v1Noise", &SidState::v1Noise)
        .field("v1DecayTime", &SidState::v1DecayTime)
        .field("v1AttackTime", &SidState::v1AttackTime)
        .field("v1ReleaseTime", &SidState::v1ReleaseTime)
        .field("v1SustainVolume", &SidState::v1SustainVolume)

        .field("v2Frequency", &SidState::v2Frequency)
        .field("v2PulseWidth", &SidState::v2PulseWidth)
        .field("v2On", &SidState::v2On)
        .field("v2Sync", &SidState::v2Sync)
        .field("v2RingMod", &SidState::v2RingMod)
        .field("v2Disable", &SidState::v2Disable)
        .field("v2Triangle", &SidState::v2Triangle)
        .field("v2Sawtooth", &SidState::v2Sawtooth)
        .field("v2Pulse", &SidState::v2Pulse)
        .field("v2Noise", &SidState::v2Noise)
        .field("v2DecayTime", &SidState::v2DecayTime)
        .field("v2AttackTime", &SidState::v2AttackTime)
        .field("v2ReleaseTime", &SidState::v2ReleaseTime)
        .field("v2SustainVolume", &SidState::v2SustainVolume)

        .field("v3Frequency", &SidState::v3Frequency)
        .field("v3PulseWidth", &SidState::v3PulseWidth)
        .field("v3On", &SidState::v3On)
        .field("v3Sync", &SidState::v3Sync)
        .field("v3RingMod", &SidState::v3RingMod)
        .field("v3Disable", &SidState::v3Disable)
        .field("v3Triangle", &SidState::v3Triangle)
        .field("v3Sawtooth", &SidState::v3Sawtooth)
        .field("v3Pulse", &SidState::v3Pulse)
        .field("v3Noise", &SidState::v3Noise)
        .field("v3DecayTime", &SidState::v3DecayTime)
        .field("v3AttackTime", &SidState::v3AttackTime)
        .field("v3ReleaseTime", &SidState::v3ReleaseTime)
        .field("v3SustainVolume", &SidState::v3SustainVolume)

        .field("cutoffFrequency", &SidState::cutoffFrequency)
        .field("filterResonance", &SidState::filterResonance)
        .field("volume", &SidState::volume)
        .field("lowPassFilter", &SidState::lowPassFilter)
        .field("bandPassFilter", &SidState::bandPassFilter)
        .field("highPassFilter", &SidState::highPassFilter)
        .field("voice1Filtered", &SidState::voice1Filtered)
        .field("voice2Filtered", &SidState::voice2Filtered)
        .field("voice3Filtered", &SidState::voice3Filtered);

    emscripten::function("getSidState", &getSidState);
}

EMSCRIPTEN_KEEPALIVE
void startEmulator() {
    std::cout << "Starting emulator" << std::endl;
    emulatorSystem.vic->setFramebufferCallback([](std::array<uint32_t, 40 * 25 * 8 * 8>& screen) {
        fbDiff.clear();

        size_t i = 0;
        while(i < screen.size()) {
            if(screen[i] != lastFramebuffer[i]) {
                size_t runStart = i;
                uint32_t color = screen[i];

                do {
                    i++;
                } while(i < screen.size() && screen[i] == color &&
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
    emulatorSystem.sid->setWriteCallback([]() { EM_ASM({ sidStateChanged(); }); });

    while(true) {
        emulatorSystem.step();
        if(paused) {
            emscripten_sleep(5);
            continue;
        }

        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;

        if(elapsed.count() >= 0.5) {
            lastTime = currentTime;

            EM_ASM({ processOtherStuff(); });
        }

        if(emulatorSystem.vic->needsRender) {
            emscripten_sleep(0);
            EM_ASM({ framebufferReady(); });
            emulatorSystem.vic->needsRender = false;
        }
    }
}
} // extern "C"
#endif

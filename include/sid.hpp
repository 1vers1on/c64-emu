#pragma once

#include <cstdint>
#include <functional>

#define SID_CLOCK_SPEED 985000
#define SAMPLE_RATE 44100

struct SidState {
    uint16_t v1Frequency;
    float v1PulseWidth;
    bool v1On;
    bool v1Sync;
    bool v1RingMod;
    bool v1Disable;
    bool v1Triangle;
    bool v1Sawtooth;
    bool v1Pulse;
    bool v1Noise;
    int v1DecayTime;
    int v1AttackTime;
    int v1ReleaseTime;
    float v1SustainVolume;

    uint16_t v2Frequency;
    float v2PulseWidth;
    bool v2On;
    bool v2Sync;
    bool v2RingMod;
    bool v2Disable;
    bool v2Triangle;
    bool v2Sawtooth;
    bool v2Pulse;
    bool v2Noise;
    int v2DecayTime;
    int v2AttackTime;
    int v2ReleaseTime;
    float v2SustainVolume;

    uint16_t v3Frequency;
    float v3PulseWidth;
    bool v3On;
    bool v3Sync;
    bool v3RingMod;
    bool v3Disable;
    bool v3Triangle;
    bool v3Sawtooth;
    bool v3Pulse;
    bool v3Noise;
    int v3DecayTime;
    int v3AttackTime;
    int v3ReleaseTime;
    float v3SustainVolume;

    uint16_t cutoffFrequency;

    uint8_t filterResonance;

    uint8_t volume;

    bool lowPassFilter;
    bool bandPassFilter;
    bool highPassFilter;

    bool voice1Filtered;
    bool voice2Filtered;
    bool voice3Filtered;
};

struct Voice {
    uint16_t frequency;
    uint16_t pulseWidthInternal;
    float pulseWidth;
    bool voiceOn;
    bool synchronization;
    bool ringModulation;
    bool disableVoice;
    bool triangleEnabled;
    bool sawtoothEnabled;
    bool pulseEnabled;
    bool noiseEnabled;
    int decayTimeMs;
    int attackTimeMs;
    int releaseTimeMs;
    float sustainVolume;
    float phaseTriangle;
    float phaseSawtooth;
    float phasePulse;
    float amplitude;
    float msSinceStarted;
};

struct Filter {
    uint16_t cutoffFrequency;
    bool voice1Filtered;
    bool voice2Filtered;
    bool voice3Filtered;
    bool externalVoiceFiltered;
    uint8_t filterResonance;
    uint8_t volume;
    bool lowPassFilter;
    bool bandPassFilter;
    bool highPassFilter;
    bool voice3ThroughFilter;
};

class SID {
public:
    SID();
    float tick();
    void write(uint16_t addr, uint8_t value);
    uint8_t read(uint16_t addr);

    SidState getState() const {
        SidState state;
        state.v1Frequency = voice1.frequency;
        state.v1PulseWidth = voice1.pulseWidth;
        state.v1On = voice1.voiceOn;
        state.v1Sync = voice1.synchronization;
        state.v1RingMod = voice1.ringModulation;
        state.v1Disable = voice1.disableVoice;
        state.v1Triangle = voice1.triangleEnabled;
        state.v1Sawtooth = voice1.sawtoothEnabled;
        state.v1Pulse = voice1.pulseEnabled;
        state.v1Noise = voice1.noiseEnabled;
        state.v1DecayTime = voice1.decayTimeMs;
        state.v1AttackTime = voice1.attackTimeMs;
        state.v1ReleaseTime = voice1.releaseTimeMs;
        state.v1SustainVolume = voice1.sustainVolume;

        state.v2Frequency = voice2.frequency;
        state.v2PulseWidth = voice2.pulseWidth;
        state.v2On = voice2.voiceOn;
        state.v2Sync = voice2.synchronization;
        state.v2RingMod = voice2.ringModulation;
        state.v2Disable = voice2.disableVoice;
        state.v2Triangle = voice2.triangleEnabled;
        state.v2Sawtooth = voice2.sawtoothEnabled;
        state.v2Pulse = voice2.pulseEnabled;
        state.v2Noise = voice2.noiseEnabled;
        state.v2DecayTime = voice2.decayTimeMs;
        state.v2AttackTime = voice2.attackTimeMs;
        state.v2ReleaseTime = voice2.releaseTimeMs;
        state.v2SustainVolume = voice2.sustainVolume;

        state.v3Frequency = voice3.frequency;
        state.v3PulseWidth = voice3.pulseWidth;
        state.v3On = voice3.voiceOn;
        state.v3Sync = voice3.synchronization;
        state.v3RingMod = voice3.ringModulation;
        state.v3Disable = voice3.disableVoice;
        state.v3Triangle = voice3.triangleEnabled;
        state.v3Sawtooth = voice3.sawtoothEnabled;
        state.v3Pulse = voice3.pulseEnabled;
        state.v3Noise = voice3.noiseEnabled;
        state.v3DecayTime = voice3.decayTimeMs;
        state.v3AttackTime = voice3.attackTimeMs;
        state.v3ReleaseTime = voice3.releaseTimeMs;
        state.v3SustainVolume = voice3.sustainVolume;

        state.cutoffFrequency = filter.cutoffFrequency;
        state.filterResonance = filter.filterResonance;
        state.volume = filter.volume;
        state.lowPassFilter = filter.lowPassFilter;
        state.bandPassFilter = filter.bandPassFilter;
        state.highPassFilter = filter.highPassFilter;
        state.voice1Filtered = filter.voice1Filtered;
        state.voice2Filtered = filter.voice2Filtered;
        state.voice3Filtered = filter.voice3Filtered;

        return state;
    }

    void setWriteCallback(std::function<void()> callback) {
        writeCallback = callback;
    }

private:
    std::function<void()> writeCallback;

    Voice voice1;
    Voice voice2;
    Voice voice3;
    Filter filter;
};

#pragma once

#include <cstdint>

#define SID_CLOCK_SPEED 985000
#define SAMPLE_RATE 44100

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

private:
    Voice voice1;
    Voice voice2;
    Voice voice3;
    Filter filter;
};

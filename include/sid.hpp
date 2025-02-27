#pragma once

#include <cstdint>
#include <vector>
#include <ASDREnvelope.hpp>

#define nsPerCycle 1015 // for pal systems
#define CLOCK_SPEED 985000

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
    // int sustainVolume;
    float sustainVolume;
    float phase;
    float amplitude;
    float msSinceStarted;
    ADSREnvelope* envelope;
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

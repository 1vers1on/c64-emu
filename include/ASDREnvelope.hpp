#pragma once

#include <iostream>
#include <cmath>

enum class EnvelopeState { idle, attack, decay, sustain, release };

class ADSREnvelope {
public:
    ADSREnvelope(float attackTimeMs, float decayTimeMs, float sustainVolume, float releaseTimeMs);

    void noteOn();
    void noteOff();

    void setAttackTime(float attackTimeMs) { this->attackTimeMs = attackTimeMs; }
    void setDecayTime(float decayTimeMs) { this->decayTimeMs = decayTimeMs; }
    void setSustainVolume(float sustainVolume) { this->sustainVolume = sustainVolume; }
    void setReleaseTime(float releaseTimeMs) { this->releaseTimeMs = releaseTimeMs; }

    float process(float msSinceStarted);

private:
    float attackTimeMs, decayTimeMs, sustainVolume, releaseTimeMs;
    float msSinceStarted;
    EnvelopeState state;
};
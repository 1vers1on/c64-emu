#include <ASDREnvelope.hpp>

ADSREnvelope::ADSREnvelope(float attackTimeMs, float decayTimeMs, float sustainVolume, float releaseTimeMs) {
    this->attackTimeMs = attackTimeMs;
    this->decayTimeMs = decayTimeMs;
    this->sustainVolume = sustainVolume;
    this->releaseTimeMs = releaseTimeMs;
    msSinceStarted = 0;
    state = EnvelopeState::idle;
}

void ADSREnvelope::noteOn() {
    state = EnvelopeState::attack;
    msSinceStarted = 0; // i need to make this start from where it left off 
}

void ADSREnvelope::noteOff() {
    state = EnvelopeState::release;
    msSinceStarted = 0;
}

float ADSREnvelope::process(float msSinceStarted) {
    switch (state) {
        case EnvelopeState::idle:
            return 0;
        case EnvelopeState::attack:
            if (msSinceStarted < attackTimeMs) {
                return msSinceStarted / attackTimeMs;
            } else {
                state = EnvelopeState::decay;
                this->msSinceStarted = 0;
                return 1;
            }
        case EnvelopeState::decay:
            if (msSinceStarted < decayTimeMs) {
                return sustainVolume + (1 - sustainVolume) * std::exp(-5.0 * (msSinceStarted / decayTimeMs));
            } else {
                state = EnvelopeState::sustain;
                this->msSinceStarted = 0;
                return sustainVolume;
            }
        case EnvelopeState::sustain:
            return sustainVolume;
        case EnvelopeState::release:
            if (msSinceStarted < releaseTimeMs) {
                return sustainVolume * std::exp(-5.0 * (msSinceStarted / releaseTimeMs));
            } else {
                state = EnvelopeState::idle;
                return 0;
            }
    }
    return 0;
}

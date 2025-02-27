#include <sid.hpp>
#include <iostream>

SID::SID() {
    voice1 = {};
    voice2 = {};
    voice3 = {};
    filter = {};

    voice1.frequency = 1000;

    voice1.envelope = new ADSREnvelope(0, 0, 0, 0);
    voice2.envelope = new ADSREnvelope(0, 0, 0, 0);
    voice3.envelope = new ADSREnvelope(0, 0, 0, 0);
}

float triangleWave(float phase) {
    return 2 * std::abs(2 * (phase - std::floor(phase + 0.5f)));
}

float sawtoothWave(float phase) {
    return 2 * (phase - std::floor(phase));
}

float pulseWave(float phase, float pulseWidth) {
    return phase < pulseWidth ? 1.0f : -1.0f;
}

float SID::tick() {
    float voice1Output = 0;
    if (!voice1.disableVoice) {
        voice1.msSinceStarted += nsPerCycle / 1000000.0f;
        voice1.phase += static_cast<float>(voice1.frequency) / CLOCK_SPEED;

        if (voice1.pulseEnabled) {
            voice1Output += pulseWave(voice1.phase, voice1.pulseWidth);
        }
        if (voice1.sawtoothEnabled) {
            voice1Output += sawtoothWave(voice1.phase);
        }
        if (voice1.triangleEnabled) {
            voice1Output += triangleWave(voice1.phase);
        }

        voice1Output *= voice1.envelope->process(voice1.msSinceStarted);
    }

    float voice2Output = 0;
    if (!voice2.disableVoice) {
        voice2.msSinceStarted += nsPerCycle / 1000000.0f;
        voice2.phase += static_cast<float>(voice2.frequency) / CLOCK_SPEED;

        if (voice2.pulseEnabled) {
            voice2Output += pulseWave(voice2.phase, voice2.pulseWidth);
        }
        if (voice2.sawtoothEnabled) {
            voice2Output += sawtoothWave(voice2.phase);
        }
        if (voice2.triangleEnabled) {
            voice2Output += triangleWave(voice2.phase);
        }

        voice2Output *= voice2.envelope->process(voice2.msSinceStarted);
    }

    float voice3Output = 0;
    if (!voice3.disableVoice) {
        voice3.msSinceStarted += nsPerCycle / 1000000.0f;
        voice3.phase += static_cast<float>(voice3.frequency) / CLOCK_SPEED;

        if (voice3.pulseEnabled) {
            voice3Output += pulseWave(voice3.phase, voice3.pulseWidth);
        }
        if (voice3.sawtoothEnabled) {
            voice3Output += sawtoothWave(voice3.phase);
        }
        if (voice3.triangleEnabled) {
            voice3Output += triangleWave(voice3.phase);
        }

        voice3Output *= voice3.envelope->process(voice3.msSinceStarted);
    }
    float finalOutput = 0;
    float filterOutput = 0;
    // just ignore the pass filters for now
    if (filter.voice1Filtered) {
        filterOutput += voice1Output;
    } else {
        finalOutput += voice1Output;
    }

    if (filter.voice2Filtered) {
        filterOutput += voice2Output;
    } else {
        finalOutput += voice2Output;
    }

    if (filter.voice3Filtered) {
        filterOutput += voice3Output;
    } else {
        finalOutput += voice3Output;
    }

    filterOutput *= filter.volume / 15.0f;
    finalOutput += filterOutput;

    return finalOutput;
}

int decodeDecayTime(uint8_t value) {
    switch (value & 0x0F) {
        case 0x0000:
            return 6;
        case 0x0001:
            return 24;
        case 0x0010:
            return 48;
        case 0x0011:
            return 72;
        case 0x0100:
            return 114;
        case 0x0101:
            return 168;
        case 0x0110:
            return 204;
        case 0x0111:
            return 240;
        case 0x1000:
            return 300;
        case 0x1001:
            return 750;
        case 0x1010:
            return 1500;
        case 0x1011:
            return 2400;
        case 0x1100:
            return 3000;
        case 0x1101:
            return 9000;
        case 0x1110:
            return 15000;
        case 0x1111:
            return 24000;
    }

    return 0;
}

int decodeAttackTime(uint8_t value) {
    switch ((value >> 4) & 0x0F) {
        case 0x0000:
            return 2;
        case 0x0001:
            return 8;
        case 0x0010:
            return 16;
        case 0x0011:
            return 24;
        case 0x0100:
            return 38;
        case 0x0101:
            return 56;
        case 0x0110:
            return 68;
        case 0x0111:
            return 80;
        case 0x1000:
            return 100;
        case 0x1001:
            return 250;
        case 0x1010:
            return 500;
        case 0x1011:
            return 800;
        case 0x1100:
            return 1000;
        case 0x1101:
            return 3000;
        case 0x1110:
            return 5000;
        case 0x1111:
            return 8000;
    }

    return 0;
}

void SID::write(uint16_t addr, uint8_t value) {
    addr &= 0x1F; // 5 bits
    std::cout << "SID write to address: " << std::hex << addr << " with data: " << static_cast<int>(value) << std::dec << std::endl;
    switch(addr) {
        case 0x00:
            voice1.frequency = (voice1.frequency & 0xFF00) | value;
            break;
        case 0x01:
            voice1.frequency = (voice1.frequency & 0x00FF) | (value << 8);
            break;
        case 0x02:
            voice1.pulseWidthInternal = (voice1.pulseWidthInternal & 0xFF00) | value;
            voice1.pulseWidth = static_cast<float>(voice1.pulseWidthInternal) / 4096.0f;
            break;
        case 0x03:
            voice1.pulseWidthInternal = (voice1.pulseWidthInternal & 0x00FF) | (value << 8);
            voice1.pulseWidth = static_cast<float>(voice1.pulseWidthInternal) / 4096.0f;
            break;
        case 0x04:
            voice1.voiceOn = value & 0x01;
            voice1.synchronization = value & 0x02;
            voice1.ringModulation = value & 0x04;
            voice1.disableVoice = value & 0x08;
            voice1.triangleEnabled = value & 0x10;
            voice1.sawtoothEnabled = value & 0x20;
            voice1.pulseEnabled = value & 0x40;
            voice1.noiseEnabled = value & 0x80;
            if (voice1.voiceOn) {
                voice1.envelope->noteOn();
            } else {
                voice1.envelope->noteOff();
            }
            break;
        case 0x05: 
            voice1.decayTimeMs = decodeDecayTime(value);
            voice1.attackTimeMs = decodeAttackTime(value);
            break;
        case 0x06:
            voice1.releaseTimeMs = decodeDecayTime(value);
            if (static_cast<float>((value >> 4) & 0x0F) == 0) {
                voice1.sustainVolume = 0.0f;
            } else {
                voice1.sustainVolume = 1.0f / static_cast<float>((value >> 4) & 0x0F);
            }
            break;

        case 0x07:
            voice2.frequency = (voice2.frequency & 0xFF00) | value;
            break;
        case 0x08:
            voice2.frequency = (voice2.frequency & 0x00FF) | (value << 8);
            break;
        case 0x09:
            voice2.pulseWidthInternal = (voice2.pulseWidthInternal & 0xFF00) | value;
            voice2.pulseWidth = static_cast<float>(voice2.pulseWidthInternal) / 4096.0f;
            break;
        case 0x0A:
            voice2.pulseWidthInternal = (voice2.pulseWidthInternal & 0x00FF) | (value << 8);
            voice2.pulseWidth = static_cast<float>(voice2.pulseWidthInternal) / 4096.0f;
            break;
        case 0x0B:
            voice2.voiceOn = value & 0x01;
            voice2.synchronization = value & 0x02;
            voice2.ringModulation = value & 0x04;
            voice2.disableVoice = value & 0x08;
            voice2.triangleEnabled = value & 0x10;
            voice2.sawtoothEnabled = value & 0x20;
            voice2.pulseEnabled = value & 0x40;
            voice2.noiseEnabled = value & 0x80;
            if (voice2.voiceOn) {
                voice2.envelope->noteOn();
            } else {
                voice2.envelope->noteOff();
            }
            break;
        case 0x0C:
            voice2.decayTimeMs = decodeDecayTime(value);
            voice2.attackTimeMs = decodeAttackTime(value);
            break;
        case 0x0D:
            voice2.releaseTimeMs = decodeDecayTime(value);
            // voice2.sustainVolume = (value >> 4) & 0x0F;
            if (static_cast<float>((value >> 4) & 0x0F) == 0) {
                voice2.sustainVolume = 0.0f;
            } else {
                voice2.sustainVolume = 1.0f / static_cast<float>((value >> 4) & 0x0F);
            }
            break;

        case 0x0E:
            voice3.frequency = (voice3.frequency & 0xFF00) | value;
            break;
        case 0x0F:
            voice3.frequency = (voice3.frequency & 0x00FF) | (value << 8);
            break;
        case 0x10:
            voice3.pulseWidthInternal = (voice3.pulseWidthInternal & 0xFF00) | value;
            voice3.pulseWidth = static_cast<float>(voice3.pulseWidthInternal) / 4096.0f;
            break;
        case 0x11:
            voice3.pulseWidthInternal = (voice3.pulseWidthInternal & 0x00FF) | (value << 8);
            voice3.pulseWidth = static_cast<float>(voice3.pulseWidthInternal) / 4096.0f;
            break;
        case 0x12:
            voice3.voiceOn = value & 0x01;
            voice3.synchronization = value & 0x02;
            voice3.ringModulation = value & 0x04;
            voice3.disableVoice = value & 0x08;
            voice3.triangleEnabled = value & 0x10;
            voice3.sawtoothEnabled = value & 0x20;
            voice3.pulseEnabled = value & 0x40;
            voice3.noiseEnabled = value & 0x80;
            if (voice3.voiceOn) {
                voice3.envelope->noteOn();
            } else {
                voice3.envelope->noteOff();
            }
            break;
        case 0x13:
            voice3.decayTimeMs = decodeDecayTime(value);
            voice3.attackTimeMs = decodeAttackTime(value);
            break;
        case 0x14:
            voice3.releaseTimeMs = decodeDecayTime(value);
            if (static_cast<float>((value >> 4) & 0x0F) == 0) {
                voice2.sustainVolume = 0.0f;
            } else {
                voice3.sustainVolume = 1.0f / static_cast<float>((value >> 4) & 0x0F);
            }
            break;

        case 0x15:
            // filter cutoff bits 0-2. next case gets bits 3-10
            filter.cutoffFrequency = (filter.cutoffFrequency & 0xFF8) | (value & 0x07);
            break;
        case 0x16:
            // filter cutoff bits 3-10
            filter.cutoffFrequency = (filter.cutoffFrequency & 0x007) | (value << 3);
            break;
        case 0x17:
            filter.voice1Filtered = value & 0x01;
            filter.voice2Filtered = value & 0x02;
            filter.voice3Filtered = value & 0x04;
            filter.externalVoiceFiltered = value & 0x08;
            filter.filterResonance = (value >> 4) & 0x0F;
            break;
        case 0x18:
            filter.volume = value & 0x0F;
            filter.lowPassFilter = value & 0x10;
            filter.bandPassFilter = value & 0x20;
            filter.highPassFilter = value & 0x40;
            filter.voice3ThroughFilter = value & 0x80;
            break;
    }
}

uint8_t SID::read(uint16_t addr) {
    addr &= 0x1F;
    std::cout << "SID read from address: " << std::hex << addr << std::dec << std::endl;
    return 0;
}
class SidProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.pulsePhasev1 = 0;
        this.frequencyv1 = 440;
        this.pulseWidthv1 = 0.5;
        this.v1TrianglePhase = 0;
        this.v1SawtoothPhase = 0;
        this.v1AttackTime = 0.1;
        this.v1DecayTime = 0.1;
        this.v1SustainLevel = 0.5;
        this.v1ReleaseTime = 0.1;
        this.v1ASDRPhase = "idle";
        this.v1ASDRStartTime = 0;
        this.v1On = false;
        this.v1ShiftRegister = 0x01;
        this.v1PulseOn = false;
        this.v1TriangleOn = false;
        this.v1SawtoothOn = false;
        this.v1NoiseOn = false;
        this.v1Enabled = false;

        this.currentFrame = 0;
    }

    static get parameterDescriptors() {
        return [
            {
                name: 'v1Frequency',
                defaultValue: 440,
                minValue: 0,
                maxValue: 20000,
            }, 
            {
                name: 'v1pulseWidth',
                defaultValue: 0.5,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1On',
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1AttackTime',
                defaultValue: 0.1,
                minValue: 0,
                maxValue: 5,
            },
            {
                name: 'v1DecayTime',
                defaultValue: 0.1,
                minValue: 0,
                maxValue: 5,
            },
            {
                name: 'v1SustainLevel',
                defaultValue: 0.5,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1ReleaseTime',
                defaultValue: 0.1,
                minValue: 0,
                maxValue: 5,
            },
            {
                name: 'v1pulseOn',
                defaultValue: 1,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1TriangleOn',
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1SawtoothOn',
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1NoiseOn',
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: 'v1Enabled',
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            }
        ];
    }

    process(inputs, outputs, parameters) {
        const output_ = outputs[0];
        const channel = output_[0];

        this.v1Frequency = parameters.v1Frequency[0];
        this.pulseWidthv1 = parameters.v1pulseWidth[0];
        this.v1AttackTime = parameters.v1AttackTime[0];
        this.v1DecayTime = parameters.v1DecayTime[0];
        this.v1SustainLevel = parameters.v1SustainLevel[0];
        this.v1ReleaseTime = parameters.v1ReleaseTime[0];
        this.v1PulseOn = parameters.v1pulseOn[0] > 0;
        this.v1TriangleOn = parameters.v1TriangleOn[0] > 0;
        this.v1SawtoothOn = parameters.v1SawtoothOn[0] > 0;
        this.v1NoiseOn = parameters.v1NoiseOn[0] > 0;
        this.v1Enabled = parameters.v1Enabled[0] > 0;

        const currentIsOn = parameters.v1On[0] > 0;

        // Check for note on/off transitions
        if (currentIsOn !== this.v1On) {
            if (this.v1On && !currentIsOn) {
                // Note off - start release phase
                this.v1ASDRPhase = "release";
                this.v1ASDRStartTime = this.currentFrame;
            } else if (!this.v1On && currentIsOn) {
                // Note on - start attack phase
                this.v1ASDRPhase = "attack";
                this.v1ASDRStartTime = this.currentFrame;
            }
            this.v1On = currentIsOn;
        }
        
        for (let i = 0; i < channel.length; i++) {
            this.currentFrame++;
            
            // Update noise shift register (only when needed)
            if (this.v1NoiseOn) {
                let feedbackBit = ((this.v1ShiftRegister >> 22) & 1) ^ ((this.v1ShiftRegister >> 17) & 1);
                this.v1ShiftRegister = (this.v1ShiftRegister << 1) | feedbackBit;
                this.v1ShiftRegister &= 0x7FFFFF; // Mask to 23 bits
            }

            const v1ElapsedTime = (this.currentFrame - this.v1ASDRStartTime) / sampleRate;
            
            let v1Amplitude = 0;
            
            if (this.v1ASDRPhase === "attack") {
                if (v1ElapsedTime < this.v1AttackTime) {
                    v1Amplitude = v1ElapsedTime / this.v1AttackTime;
                } else {
                    v1Amplitude = 1.0;
                    this.v1ASDRPhase = "decay";
                    this.v1ASDRStartTime = this.currentFrame;
                }
            } else if (this.v1ASDRPhase === "decay") {
                if (v1ElapsedTime < this.v1DecayTime) {
                    v1Amplitude = 1.0 - ((v1ElapsedTime / this.v1DecayTime) * (1.0 - this.v1SustainLevel));
                } else {
                    v1Amplitude = this.v1SustainLevel;
                    this.v1ASDRPhase = "sustain";
                }
            } else if (this.v1ASDRPhase === "sustain") {
                v1Amplitude = this.v1SustainLevel;
            } else if (this.v1ASDRPhase === "release") {
                if (v1ElapsedTime < this.v1ReleaseTime) {
                    v1Amplitude = this.v1SustainLevel * (1.0 - (v1ElapsedTime / this.v1ReleaseTime));
                } else {
                    v1Amplitude = 0;
                    this.v1ASDRPhase = "idle";
                }
            }

            let v1Output = 0;
            let v1ActiveWaveforms = 0;
            
            this.pulsePhasev1 += (this.v1Frequency / sampleRate);
            if (this.pulsePhasev1 >= 1) {
                this.pulsePhasev1 -= 1;
            }
            
            this.v1TrianglePhase += (this.v1Frequency / sampleRate);
            if (this.v1TrianglePhase >= 1) {
                this.v1TrianglePhase -= 1;
            }
            
            this.v1SawtoothPhase += (this.v1Frequency / sampleRate);
            if (this.v1SawtoothPhase >= 1) {
                this.v1SawtoothPhase -= 1;
            }
            
            if (this.v1PulseOn) {
                v1ActiveWaveforms++;
                if (this.pulsePhasev1 < this.pulseWidthv1) {
                    v1Output += 0.5;
                } else {
                    v1Output -= 0.5;
                }
            }
            
            if (this.v1TriangleOn) {
                v1ActiveWaveforms++;
                if (this.v1TrianglePhase < 0.5) {
                    v1Output += 2 * this.v1TrianglePhase - 0.5;
                } else {
                    v1Output += 1.5 - 2 * this.v1TrianglePhase;
                }
            }
            
            if (this.v1SawtoothOn) {
                v1ActiveWaveforms++;
                v1Output += this.v1SawtoothPhase - 0.5;
            }
            
            if (this.v1NoiseOn) {
                v1ActiveWaveforms++;
                v1Output += ((this.v1ShiftRegister & 1) ? 0.5 : -0.5);
            }
            
            if (v1ActiveWaveforms > 1) {
                v1Output /= v1ActiveWaveforms;
            }

            v1Output *= v1Amplitude;
            
            channel[i] = v1Output;
        }

        return true;
    }
}

registerProcessor('sid-processor', SidProcessor);

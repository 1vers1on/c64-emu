class SidFilterProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.cutoff = 1000;
        this.resonance = 0.4;
        this.filterMode = "bandpass";
        this.volume = 1.0;

        this.lp = 0;
        this.bp = 0;
    }

    static get parameterDescriptors() {
        return [
            {
                name: "cutoff",
                defaultValue: 1000,
                minValue: 0,
                maxValue: 20000,
            },
            {
                name: "resonance",
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: "enabled",
                defaultValue: 0,
                minValue: 0,
                maxValue: 1,
            },
            {
                name: "volume",
                defaultValue: 1.0,
                minValue: 0,
                maxValue: 1,
            },
        ];
    }

    process(inputs, outputs, parameters) {
        this.cutoff = parameters.cutoff[0];
        this.resonance = parameters.resonance[0];
        this.volume = parameters.volume[0];
        this.filterMode = parameters.enabled[0] > 0 ? "bandpass" : "off";

        const input = inputs[0];
        const output = outputs[0];

        if (input.length === 0) return true;

        const f = Math.min(1.0, this.cutoff / (sampleRate / 2));
        const q = 1.0 - this.resonance;

        for (let channel = 0; channel < input.length; channel++) {
            const inputChannel = input[channel];
            const outputChannel = output[channel];

            for (let i = 0; i < inputChannel.length; i++) {
                const x = inputChannel[i];

                this.bp += f * (x - this.lp - q * this.bp);
                this.lp += f * this.bp;
                const hp = x - this.lp - q * this.bp;
                const notch = hp + this.lp;

                switch (this.filterMode) {
                    case "lowpass":
                        outputChannel[i] = this.lp;
                        break;
                    case "highpass":
                        outputChannel[i] = hp;
                        break;
                    case "bandpass":
                        outputChannel[i] = this.bp;
                        break;
                    case "notch":
                        outputChannel[i] = notch;
                        break;
                    default:
                        outputChannel[i] = x; // Pass-through
                }
            }

            for (let i = 0; i < outputChannel.length; i++) {
                outputChannel[i] *= this.volume;
            }
        }

        return true;
    }
}

registerProcessor("sid-filter-processor", SidFilterProcessor);
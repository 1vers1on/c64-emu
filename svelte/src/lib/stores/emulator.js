// This store will manage the state of the emulator across components

import { writable } from 'svelte/store';

// Create a store for console output
export const consoleOutput = writable("");

// Create a store for emulator settings
export const emulatorSettings = writable({
    scanlines: false,
    pixelPerfect: true,
    soundEnabled: true,
    autoFrame: false,
    volume: 75,
    speed: 100
});

// Function to add output to the console
export function addToConsole(text, isError = false) {
    consoleOutput.update(current => {
        if (isError) {
            return current + `<span style="color: #ff5555;">${text}</span>\n`;
        }
        return current + text + '\n';
    });
}

// Function to clear the console
export function clearConsole() {
    consoleOutput.set("");
}

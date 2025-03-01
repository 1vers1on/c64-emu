import { writable } from 'svelte/store';
import type { Writable } from 'svelte/store';

export interface Settings {
    scanlines: boolean;
    soundEnabled: boolean;
    volume: number;
    speed: number;
}

// Create a store for console output
export const consoleOutput = writable("");

// Create a store for emulator settings
export const emulatorSettings: Writable<Settings> = writable({
    scanlines: false,
    soundEnabled: true,
    volume: 75,
    speed: 100
});

// Function to add output to the console
export function addToConsole(text: string, isError = false) {
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

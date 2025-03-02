<script lang="ts">
    // craete scanblines store
    import { writable } from "svelte/store";

    import ToggleSwitch from "./ToggleSwitch.svelte";
    import SliderControl from "./SliderControl.svelte";
    import { emulatorSettings } from "$lib/stores/emulator";

    // let scanlines = false;
    const scanlines = writable(false);

    // listen for changes in emulatorSettings

    $: console.log("emulatorSettings", $emulatorSettings);
</script>

<div class="glass-panel settings-container">
    <div class="panel-header">
        <div class="panel-title">Settings</div>
        <div>
            <i class="fas fa-sliders-h"></i>
        </div>
    </div>

    <div class="setting-group">
        <ToggleSwitch label="CRT Scanlines" bind:active={$emulatorSettings.scanlines} />
        <ToggleSwitch
            label="Sound Enabled"
            bind:active={(emulatorSettings as any).soundEnabled}
        />
    </div>

    <SliderControl
        label="Volume"
        bind:value={(emulatorSettings as any).volume}
        min={0}
        max={100}
        unit="%"
    />

    <SliderControl
        label="Emulation Speed"
        bind:value={(emulatorSettings as any).speed}
        min={50}
        max={200}
        unit="%"
    />
</div>

<style>
    .settings-container {
        display: flex;
        flex-direction: column;
        gap: 1rem;
    }

    .setting-group {
        margin-top: -0.5rem;
    }
</style>

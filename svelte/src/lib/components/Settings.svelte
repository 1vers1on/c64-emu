<script>
    import ToggleSwitch from "./ToggleSwitch.svelte";
    import SliderControl from "./SliderControl.svelte";

    // Initial settings state
    let settings = {
        scanlines: false,
        soundEnabled: true,
        volume: 75,
        speed: 100,
    };
</script>

<div class="glass-panel settings-container">
    <div class="panel-header">
        <div class="panel-title">Settings</div>
        <div>
            <i class="fas fa-sliders-h"></i>
        </div>
    </div>

    <div class="setting-group">
        <ToggleSwitch label="CRT Scanlines" bind:active={settings.scanlines} />

        <ToggleSwitch
            label="Sound Enabled"
            bind:active={settings.soundEnabled}
        />
    </div>

    <SliderControl
        label="Volume"
        bind:value={settings.volume}
        min={0}
        max={100}
        unit="%"
    />

    <SliderControl
        label="Emulation Speed"
        bind:value={settings.speed}
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
        margin-top: 0.5rem;
    }
</style>

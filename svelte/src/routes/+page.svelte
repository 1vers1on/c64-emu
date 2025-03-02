<script lang="ts">
    import Header from "$lib/components/Header.svelte";
    import EmulatorDisplay from "$lib/components/EmulatorDisplay.svelte";
    import AdditionalControls from "$lib/components/AdditionalControls.svelte";
    import ConsoleOutput from "$lib/components/ConsoleOutput.svelte";
    import Settings from "$lib/components/Settings.svelte";
    import { addToConsole } from "$lib/stores/emulator.js";
</script>

<div class="container">
    <Header />

    <div class="grid">
        <main>
            <EmulatorDisplay on:message={(e) => addToConsole(e.detail.text)} />
            <AdditionalControls />
        </main>

        <aside class="sidebar">
            <ConsoleOutput />
            <Settings />
        </aside>
    </div>

    <div class="footer">
        <span style="opacity: 0.7;">1vers1on &copy; 2025</span> | <span>🏳️‍⚧️</span>
    </div>
</div>

<style>
    .sidebar {
        display: flex;
        flex-direction: column;
        gap: 1.5rem;
    }
</style>

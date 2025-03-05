<script lang="ts">
    import { run } from 'svelte/legacy';

    import { onMount } from "svelte";
    import { consoleOutput } from "$lib/stores/emulator.js";

    export function handleMessage(message: { text: string; error?: boolean }) {
        if (message.error) {
            $consoleOutput += `<span style="color: #ff5555;">${message.text}</span>\n`;
        } else {
            $consoleOutput += `${message.text}\n`;
        }
    }

    function clearOutput() {
        $consoleOutput = "";
    }

    let outputElement: HTMLDivElement = $state();
    // run(() => {
    //     if (outputElement && $consoleOutput) {
    //         setTimeout(() => {
    //             outputElement.scrollTop = outputElement.scrollHeight;
    //         }, 0);
    //     }
    // });

    onMount(() => {
    });
</script>

<div class="glass-panel output-container">
    <div class="panel-header">
        <div class="panel-title">Console Output</div>
        <button
            class="btn btn-secondary"
            style="padding: 0.2rem 0.5rem;"
            onclick={clearOutput}
            aria-label="Clear console output"
        >
            <i class="fas fa-trash-alt"></i>
        </button>
    </div>
    <div
        bind:this={outputElement}
        id="output"
        contenteditable="false"
        bind:innerHTML={$consoleOutput}
    ></div>
</div>

<style>
    .output-container {
        display: flex;
        flex-direction: column;
        height: 300px;
    }

    #output {
        flex: 1;
        background: rgba(0, 0, 0, 0.3);
        border-radius: 10px;
        border: 1px solid var(--glass-border);
        padding: 1rem;
        font-family: "Courier New", monospace;
        color: #50fa7b;
        overflow-y: auto;
        margin-top: 0.5rem;
        white-space: pre-wrap;
        font-size: 0.85rem;
    }
</style>

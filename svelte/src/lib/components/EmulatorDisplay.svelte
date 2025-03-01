<script lang="ts">
    import { onMount, onDestroy } from "svelte";
    import { addToConsole, clearConsole } from "$lib/stores/emulator";

    let canvas: HTMLCanvasElement;
    let ctx: CanvasRenderingContext2D;
    let worker: Worker;
    let outputStore;

    function start() {
        console.log("Starting emulator...");
        worker.postMessage({ type: "start" });
        addToConsole("Starting emulator...");
    }

    function renderFrame() {
        worker.postMessage({ type: "requestFrame" });
    }

    function pause() {
        console.log("Emulation paused");
        addToConsole("Emulation paused");
    }

    function reset() {
        console.log("Emulator reset");
        clearConsole();
    }

    function screenshot() {
        const image = canvas.toDataURL("image/png");

        const link = document.createElement("a");
        link.href = image;
        link.download = "commodore64-screenshot.png";
        link.click();

        console.log("Screenshot saved");
        addToConsole("Screenshot saved");
    }

    onMount(() => {
        const context = canvas.getContext("2d");
        if (context) {
            ctx = context;
        } else {
            console.error("Failed to get 2D context");
        }

        worker = new Worker("/worker.js", { type: "module" });

        worker.onmessage = (e) => {
            if (e.data.type === "print") {
                console.log(e.data.text);
                addToConsole(e.data.text);
            } else if (e.data.type === "printErr") {
                addToConsole(e.data.text, true);
            } else if (e.data.type === "frame") {
                var fb = new Uint32Array(e.data.framebuffer);
                var imgData = ctx.createImageData(320, 200);

                for (let i = 0; i < fb.length; i++) {
                    let pixel = fb[i];
                    let offset = i * 4;
                    imgData.data[offset] = (pixel >> 16) & 0xff; // red
                    imgData.data[offset + 1] = (pixel >> 8) & 0xff; // green
                    imgData.data[offset + 2] = pixel & 0xff; // blue
                    imgData.data[offset + 3] = 0xff; // alpha
                }

                ctx.putImageData(imgData, 0, 0);
            }
        };
    });

    onDestroy(() => {
        if (worker) {
            worker.terminate();
        }
    });
</script>

<div class="glass-panel emulator-container glow">
    <div class="panel-header">
        <div class="panel-title">Emulator Display</div>
        <div>
            <i class="fas fa-expand-arrows-alt"></i>
        </div>
    </div>

    <canvas bind:this={canvas} id="canvas" width="320" height="200" style="image-rendering: pixelated;"></canvas>

    <div class="controls">
        <button class="btn" on:click={start}>
            <i class="fas fa-play"></i> Start
        </button>
        <button class="btn" on:click={renderFrame}>
            <i class="fas fa-sync-alt"></i> Render Frame
        </button>
        <button class="btn btn-secondary" on:click={pause}>
            <i class="fas fa-pause"></i> Pause
        </button>
        <button class="btn btn-secondary" on:click={reset}>
            <i class="fas fa-redo"></i> Reset
        </button>
        <button class="btn btn-secondary" on:click={screenshot}>
            <i class="fas fa-camera"></i> Screenshot
        </button>
    </div>
</div>

<style>
    .emulator-container {
        position: relative;
        overflow: hidden;
    }

    #canvas {
        width: 100%;
        height: auto;
        background-color: var(--darker);
        border-radius: 10px;
        display: block;
        margin: 0 auto;
        border: 1px solid var(--glass-border);
    }
</style>

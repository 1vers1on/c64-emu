<script>
    import { onMount, onDestroy } from "svelte";
    import { createEventDispatcher } from "svelte";
    const dispatch = createEventDispatcher();

    let canvas;
    let ctx;
    let worker;
    let outputStore;

    function start() {
        console.log("Starting emulator...");
        worker.postMessage({ type: "start" });
        dispatch("message", { text: "Starting emulator..." });
    }

    function renderFrame() {
        worker.postMessage({ type: "requestFrame" });
    }

    function pause() {
        console.log("Emulation paused");
        dispatch("message", { text: "Emulation paused" });
    }

    function reset() {
        console.log("Emulator reset");
        dispatch("message", {
            text: "",
        });
    }

    function screenshot() {
        const image = canvas.toDataURL("image/png");

        const link = document.createElement("a");
        link.href = image;
        link.download = "commodore64-screenshot.png";
        link.click();

        console.log("Screenshot saved");
        dispatch("message", { text: "Screenshot saved" });
    }

    onMount(() => {
        ctx = canvas.getContext("2d");

        worker = new Worker("/worker.js", { type: "module" });

        worker.onmessage = (e) => {
            if (e.data.type === "print") {
                console.log(e.data.text);
                dispatch("message", { text: e.data.text });
            } else if (e.data.type === "printErr") {
                dispatch("message", { text: e.data.text, error: true });
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

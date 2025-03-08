<script lang="ts">
    import { onMount, onDestroy } from "svelte";
    import { addToConsole, clearConsole, emulatorSettings } from "$lib/stores/emulator";

    let canvas: HTMLCanvasElement;
    let ctx: CanvasRenderingContext2D;
    let worker: Worker;
    let outputStore;
    let textInput: HTMLInputElement;

    let clockSpeed = $state(0);

    let audioContext: AudioContext;

    function screenshot() {
        const image = canvas.toDataURL("image/png");

        const link = document.createElement("a");
        link.href = image;
        link.download = "commodore64-screenshot.png";
        link.click();

        console.log("Screenshot saved");
        addToConsole("Screenshot saved");
    }

    function reverseKeyInput(key: string, shiftPressed: boolean): string {
        if (key === "Shift") {
            return "LSHIFT";
        }

        if (key === "Control") {
            return "LCTRL";
        }
        if (key === "Backspace") {
            return "DELETE";
        }
        if (key === '"') {
            return "2";
        }

        if (key === "Enter") {
            return "RETURN";
        }

        if (key === " ") {
            return "SPACE";
        }

        if (shiftPressed) {
            switch (key) {
                case "!":
                    return "1";
                // case "@":
                //     return "";
                case "#":
                    return "3";
                case "$":
                    return "4";
                case "%":
                    return "5";
                // case "^":
                //     return "6";
                case "&":
                    return "6";
                case "*":
                    return "*";
                case "(":
                    return "8";
                case ")":
                    return "9";
                case "_":
                    return "-";
                case "+":
                    return "+";
                case "{":
                    return "[";
                case "}":
                    return "]";
                case "|":
                    return "\\";
                case ":":
                    return "{";
                case '"':
                    return "'";
                case "<":
                    return ",";
                case ">":
                    return ".";
                case "?":
                    return "/";
                case "~":
                    return "`";
            }
        }
        return key.toUpperCase();
    }

    function onKeyDown(event: KeyboardEvent) {
        worker.postMessage({
            type: "keyDown",
            key: reverseKeyInput(event.key, event.shiftKey),
        });
    }

    function onKeyUp(event: KeyboardEvent) {
        worker.postMessage({
            type: "keyUp",
            key: reverseKeyInput(event.key, event.shiftKey),
        });
    }

    function asciiToPetscii(input: string): Uint8Array {
        const petsciiMap: Record<number, number> = {
            // lowercase to uppercase (petscii lacks lowercase in standard mode)
            ...Object.fromEntries(
                [...Array(26)].map((_, i) => [97 + i, 65 + i]),
            ), // 'a'->'A', 'b'->'B', etc.

            // reverse mappings for some common ascii characters
            91: 160, // '[' -> '£'
            92: 161, // '\' -> '⎻'
            93: 162, // ']' -> '⎼'
            94: 94, // '^' stays the same
            95: 95, // '_' stays the same
            123: 185, // '{' -> 'π'
            124: 186, // '|' -> '┤'
            125: 187, // '}' -> '┐'
            126: 188, // '~' -> '└'
        };

        return new Uint8Array(
            input.split("").map((char) => {
                const code = char.charCodeAt(0);
                if (code in petsciiMap) return petsciiMap[code];
                if (code >= 32 && code <= 95) return code; // basic ascii range
                if (code >= 192 && code <= 223) return code - 128; // extended petscii range
                return 32; // unknown characters become space
            }),
        );
    }

    onMount(() => {
        textInput.addEventListener("keydown", (event) => {
            if (event.key === "Enter") {
                event.preventDefault();
                worker.postMessage({
                    type: "inputText",
                    bytes: asciiToPetscii(textInput.value),
                });
                textInput.value = "";
            }
        });

        canvas.addEventListener("keydown", onKeyDown);
        canvas.addEventListener("keyup", onKeyUp);

        window.addEventListener("keydown", (event) => {
            if (event.key === " " || event.key === "Tab") {
                event.preventDefault();
            }
        });

        worker = new Worker("/worker.js", { type: "module" });

        const gl =
            (canvas.getContext("webgl2") as WebGL2RenderingContext) || null;

        if (!(gl instanceof WebGL2RenderingContext)) {
            console.error("WebGL2 not supported");
            return;
        }

        function resizeCanvasToDisplaySize(canvas: HTMLCanvasElement) {
            const displayWidth = canvas.clientWidth;
            const displayHeight = canvas.clientHeight;

            if (
                canvas.width !== displayWidth ||
                canvas.height !== displayHeight
            ) {
                canvas.width = displayWidth;
                canvas.height = displayHeight;
            }
        }

        let texture: WebGLTexture;
        let program: WebGLProgram;
        let positionBuffer: WebGLBuffer;

        function initWebGL() {
            texture = gl.createTexture();
            gl.bindTexture(gl.TEXTURE_2D, texture);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            gl.texParameteri(
                gl.TEXTURE_2D,
                gl.TEXTURE_WRAP_S,
                gl.CLAMP_TO_EDGE,
            );
            gl.texParameteri(
                gl.TEXTURE_2D,
                gl.TEXTURE_WRAP_T,
                gl.CLAMP_TO_EDGE,
            );

            gl.texStorage2D(gl.TEXTURE_2D, 1, gl.RGBA8, 320, 200);

            const vertexShaderSource = `#version 300 es
                in vec2 a_position;
                in vec2 a_texCoord;
                out vec2 v_texCoord;
                void main() {
                    gl_Position = vec4(a_position, 0, 1);
                    v_texCoord = a_texCoord;
                }
            `;

            const fragmentShaderSource = `#version 300 es
                precision mediump float;

                uniform sampler2D u_image;
                uniform vec2 u_resolution;
                uniform bool u_crtEnabled;
                in vec2 v_texCoord;
                out vec4 outColor;

                #define BLOOM 1
                #define CURVATURE 1
                #define BLUR 1
                #define BLACKLEVEL 1
                #define SCANLINES 1
                // 1: shadow mask
                // 2: aperature grille
                #define SHADOW_MASK 1
                #define SHADOW_MASK_DARK 0.8
                #define VIGNETTE 1

                #define BLOOM_OFFSET            0.00015
                #define BLOOM_STRENGTH          0.8

                #define BLUR_MULTIPLIER         1.05
                #define BLUR_STRENGTH           0.2
                #define BLUR_OFFSET             0.003

                #define BLACKLEVEL_FLOOR        TINT_COLOR / 40.0

                #define CURVE_INTENSITY         1.0

                #define SHADOW_MASK_STRENGTH    0.15

                #define VIGNETTE_STRENGTH       1.2

                #define TINT_COLOR              TINT_AMBER

                #define TINT_AMBER              vec3(1.0, 0.7, 0.0) // P3 phosphor
                #define TINT_LIGHT_AMBER        vec3(1.0, 0.8, 0.0)
                #define TINT_GREEN_1            vec3(0.2, 1.0, 0.0)
                #define TINT_APPLE_II           vec3(0.2, 1.0, 0.2) // P1 phosphor
                #define TINT_GREEN_2            vec3(0.0, 1.0, 0.2)
                #define TINT_APPLE_IIc          vec3(0.4, 1.0, 0.4) // P24 phpsphor
                #define TINT_GREEN_3            vec3(0.0, 1.0, 0.4)
                #define TINT_WARM               vec3(1.0, 0.9, 0.8)
                #define TINT_COOL               vec3(0.8, 0.9, 1.0)

                #define saturate(x) clamp(x, 0.0, 1.0)
                    float ToSrgb1(float c){return(c<0.0031308?c*12.92:1.055*pow(c,0.41666)-0.055);}
                    vec4 ToSrgb(vec4 c){return vec4(ToSrgb1(c.r),ToSrgb1(c.g),ToSrgb1(c.b),ToSrgb1(c.a));}

                vec4 gammaCorrect(vec4 color) {
                    return vec4(
                        pow(color.r, 1.0 / 2.2),
                        pow(color.g, 1.0 / 2.2),
                        pow(color.b, 1.0 / 2.2),
                        color.a
                    );
                }
                vec4 getTexture(vec2 uv) {
                    vec2 textureDimensions = vec2(320.0, 200.0);
                    float scaleFactor = 0.5;
                    vec2 scaledDimensions = textureDimensions / scaleFactor;
                    vec2 offset = (u_resolution - scaledDimensions) * 0.5;
                    
                    vec2 fragCoord = uv * u_resolution;
                    vec2 centeredCoord = fragCoord - offset;
                    vec2 scaledCoord = centeredCoord * scaleFactor;
                    
                    if (scaledCoord.x >= 0.0 && scaledCoord.x < textureDimensions.x && 
                        scaledCoord.y >= 0.0 && scaledCoord.y < textureDimensions.y) {
                        return texelFetch(u_image, ivec2(scaledCoord), 0);
                    } else {
                        return vec4(0.48627450980392156, 0.4392156862745098, 0.8549019607843137, 1.0);
                    }
                }

                float blurWeights[9] = float[](0.0, 0.092, 0.081, 0.071, 0.061, 0.051, 0.041, 0.031, 0.021);

                #ifdef BLOOM
                vec3 bloom(vec3 color, vec2 uv) {
                    vec3 bloom = color - getTexture(uv + vec2(-BLOOM_OFFSET, 0)).rgb;
                    vec3 bloomMask = bloom * BLOOM_STRENGTH;
                    
                    return saturate(color + bloomMask);
                }
                #endif

                #ifdef CURVATURE
                vec2 transformCurve(vec2 uv) {
                    uv -= 0.5;
                    float r = (uv.x * uv.x + uv.y * uv.y) * CURVE_INTENSITY;
                    uv *= 4.2 + r;
                    uv *= 0.25;
                    uv += 0.5;
                    
                    return uv;
                }
                #endif

                #ifdef BLUR
                vec3 blurH(vec3 color, vec2 uv) {
                    vec3 screen = getTexture(uv).rgb * 0.102;
                    for (int i = 1; i < 9; i++) {
                        screen += getTexture(uv + vec2(float(i) * BLUR_OFFSET, 0)).rgb * blurWeights[i];
                    }
                    
                    for (int i = 1; i < 9; i++) {
                        screen += getTexture(uv + vec2(float(-i) * BLUR_OFFSET, 0)).rgb * blurWeights[i];
                    }

                    return screen * BLUR_MULTIPLIER;
                }

                vec3 blurV(vec3 color, vec2 uv) {
                    vec3 screen = getTexture(uv).rgb * 0.102;
                    for (int i = 1; i < 9; i++) {
                        screen += getTexture(uv + vec2(0, float(i) * BLUR_OFFSET)).rgb * blurWeights[i];
                    }
                    
                    for (int i = 1; i < 9; i++) {
                        screen += getTexture(uv + vec2(0, float(-i) * BLUR_OFFSET)).rgb * blurWeights[i];
                    }

                    return screen * BLUR_MULTIPLIER;
                }

                vec3 blur(vec3 color, vec2 uv) {
                    vec3 blur = (blurH(color, uv) + blurV(color, uv)) / 2.0 - color;
                    vec3 blurMask = blur * BLUR_STRENGTH;
                    return saturate(color + blurMask);
                }
                #endif

                #ifdef BLACKLEVEL
                vec3 blacklevel(vec3 color) {
                    color -= BLACKLEVEL_FLOOR;
                    color = saturate(color);
                    color += BLACKLEVEL_FLOOR;
                    return color;
                }
                #endif

                #ifdef SHADOW_MASK
                vec3 shadowMask(vec2 uv, vec2 outputSize) {
                    uv *= outputSize.xy;
                    #if SHADOW_MASK == 1
                    uv.x += uv.y * 3.0;
                    vec3 mask = vec3(SHADOW_MASK_DARK);
                    float x = fract(uv.x * (1.0 / 6.0));
                    
                    if(x < (1.0 / 3.0)) mask.r = 1.0;
                    else if(x < (2.0 / 3.0)) mask.g = 1.0;
                    else mask.b = 1.0;
                    
                    return mask;
                    #elif SHADOW_MASK == 2
                    vec3 mask = vec3(1.0);
                    float x = fract(uv.x * (1.0 / 3.0));
                    
                    if(x < (1.0 / 3.0)) mask.r = SHADOW_MASK_DARK;
                    else if(x < (2.0 / 3.0)) mask.g = SHADOW_MASK_DARK;
                    else mask.b = SHADOW_MASK_DARK;
                    
                    return mask;
                    #endif
                }
                #endif

                vec3 crt(vec2 uv, vec2 outputSize) {
                    #ifdef CURVATURE
                    uv = transformCurve(uv);

                    if (uv.x < -0.0 || uv.y < -0.0 || uv.x > 1.0 || uv.y > 1.0) {
                        return BLACKLEVEL_FLOOR;
                    }
                    #endif
                    
                    vec3 col = getTexture(uv).rgb;
                    
                    #ifdef BLOOM
                    col = bloom(col, uv);
                    #endif
                    
                    #ifdef BLUR
                    col = blur(col, uv);
                    #endif
                    
                    #ifdef BLACKLEVEL
                    col = blacklevel(col);
                    #endif
                    
                    #ifdef SCANLINES
                    float s = 1.0 - smoothstep(320.0, 1440.0, outputSize.y) + 1.0;
                    float j = cos(uv.y * outputSize.y * s) * 0.1;
                    col = col - col * j;
                    col *= 1.0 - (0.01 + ceil(mod((uv.x + 0.5) * outputSize.x, 3.0)) * (0.995 - 1.01));
                    #endif
                    
                    #ifdef SHADOW_MASK
                    col *= shadowMask(uv, outputSize);
                    #endif
                    
                    
                    #ifdef VIGNETTE
                    vec2 vigUV = uv * (1.0 - uv.yx);
                    float vignette = pow(vigUV.x * vigUV.y * 15.0, 0.25) * VIGNETTE_STRENGTH;
                    col *= vignette;
                    #endif
                    
                    return col;
                }

                void main() {
                    if (u_crtEnabled) {
                        outColor = vec4(crt(v_texCoord, u_resolution), 1.0);
                    } else {
                        outColor = getTexture(v_texCoord);
                    }
                }
            `;

            const vertexShader = compileShader(
                gl,
                gl.VERTEX_SHADER,
                vertexShaderSource,
            );
            const fragmentShader = compileShader(
                gl,
                gl.FRAGMENT_SHADER,
                fragmentShaderSource,
            );

            if (!vertexShader || !fragmentShader) {
                console.error("Failed to compile shaders");
                return;
            }

            program = gl.createProgram();
            gl.attachShader(program, vertexShader);
            gl.attachShader(program, fragmentShader);
            gl.linkProgram(program);

            if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
                console.error(
                    "Shader program error:",
                    gl.getProgramInfoLog(program),
                );
                return;
            }

            const positionLocation = gl.getAttribLocation(
                program,
                "a_position",
            );
            const texCoordLocation = gl.getAttribLocation(
                program,
                "a_texCoord",
            );

            positionBuffer = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
            gl.bufferData(
                gl.ARRAY_BUFFER,
                new Float32Array([-1, -1, 1, -1, -1, 1, 1, 1]),
                gl.STATIC_DRAW,
            );

            const texCoordBuffer = gl.createBuffer();
            gl.bindBuffer(gl.ARRAY_BUFFER, texCoordBuffer);
            gl.bufferData(
                gl.ARRAY_BUFFER,
                new Float32Array([0, 1, 1, 1, 0, 0, 1, 0]),
                gl.STATIC_DRAW,
            );

            gl.enableVertexAttribArray(positionLocation);
            gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
            gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);

            gl.enableVertexAttribArray(texCoordLocation);
            gl.bindBuffer(gl.ARRAY_BUFFER, texCoordBuffer);
            gl.vertexAttribPointer(texCoordLocation, 2, gl.FLOAT, false, 0, 0);
        }

        function compileShader(
            gl: WebGL2RenderingContext,
            type: number,
            source: string,
        ) {
            const shader = gl.createShader(type);
            if (!shader) {
                console.error("Failed to create shader");
                return null;
            }
            gl.shaderSource(shader, source);
            gl.compileShader(shader);

            if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
                console.error(
                    "Shader compile error:",
                    gl.getShaderInfoLog(shader),
                );
                gl.deleteShader(shader);
                return null;
            }

            return shader;
        }

        initWebGL();

        resizeCanvasToDisplaySize(canvas);
        gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);

        let frame = new Uint32Array(320 * 200);

        worker.onmessage = (e) => {
            if (e.data.type === "print") {
                console.log(e.data.text);
                addToConsole(e.data.text);
            } else if (e.data.type === "printErr") {
                addToConsole(e.data.text, true);
            } else if (e.data.type === "frame") {
                const fb = new Uint32Array(e.data.framebuffer);
                
                const imgData = new Uint8Array(fb.length * 4);

                for (let i = 0; i < fb.length; i++) {
                    let pixel = fb[i];
                    let offset = i * 4;
                    imgData[offset] = (pixel >> 16) & 0xff;
                    imgData[offset + 1] = (pixel >> 8) & 0xff;
                    imgData[offset + 2] = pixel & 0xff;
                    imgData[offset + 3] = 0xff;
                }

                const resolutionLocation = gl.getUniformLocation(
                    program,
                    "u_resolution",
                );
                const height = gl.canvas.height;
                const width = gl.canvas.width;
                gl.uniform2f(resolutionLocation, width, height);

                const crtEnabledLocation = gl.getUniformLocation(
                    program,
                    "u_crtEnabled",
                );
                gl.uniform1i(crtEnabledLocation, $emulatorSettings.scanlines ? 1 : 0);

                gl.bindTexture(gl.TEXTURE_2D, texture);
                gl.texSubImage2D(
                    gl.TEXTURE_2D,
                    0,
                    0,
                    0,
                    320,
                    200,
                    gl.RGBA,
                    gl.UNSIGNED_BYTE,
                    imgData,
                );

                gl.useProgram(program);
                gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
            } else if (e.data.type === "frameDiff") {   
                const diff = new Uint32Array(e.data.diff);
                const diffLength = diff.length;
                let i = 0;
                while (i < diffLength) {
                    const runStart = diff[i];
                    const runLength = diff[i + 1];
                    const color = diff[i + 2];


                    if (runStart + runLength <= frame.length) {
                        for (let j = runStart; j < runStart + runLength; j++) {
                            frame[j] = color;
                        }
                    } else {
                        console.error(`Invalid frame update range: start=${runStart}, length=${runLength}, frameSize=${frame.length}`);
                    }
                    i += 3;
                }
                
                const imgData = new Uint8Array(frame.length * 4);

                for (let i = 0; i < frame.length; i++) {
                    let pixel = frame[i];
                    let offset = i * 4;
                    imgData[offset] = (pixel >> 16) & 0xff;
                    imgData[offset + 1] = (pixel >> 8) & 0xff;
                    imgData[offset + 2] = pixel & 0xff;
                    imgData[offset + 3] = 0xff;
                }

                const resolutionLocation = gl.getUniformLocation(
                    program,
                    "u_resolution",
                );
                const height = gl.canvas.height;
                const width = gl.canvas.width;
                gl.uniform2f(resolutionLocation, width, height);

                const crtEnabledLocation = gl.getUniformLocation(
                    program,
                    "u_crtEnabled",
                );
                gl.uniform1i(crtEnabledLocation, $emulatorSettings.scanlines ? 1 : 0);

                gl.bindTexture(gl.TEXTURE_2D, texture);
                gl.texSubImage2D(
                    gl.TEXTURE_2D,
                    0,
                    0,
                    0,
                    320,
                    200,
                    gl.RGBA,
                    gl.UNSIGNED_BYTE,
                    imgData,
                );

                gl.useProgram(program);
                gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
            } else if (e.data.type === "updateInfo") {
                clockSpeed = e.data.clockSpeed;
            } else if (e.data.type === "saveState") {
                const blob = new Blob([e.data.memory], { type: "application/octet-stream" });
                const url = URL.createObjectURL(blob);
                const link = document.createElement("a");
                link.style.display = "none";
                link.href = url;
                link.download = "commodore64-state.bin";
                link.click();
                link.remove();
                URL.revokeObjectURL(url);
            }
        };

        audioContext = new (window.AudioContext || (window as any).webkitAudioContext)();
    });
    function test() {
        worker.postMessage({
            type: "test",
        });
    }

    function reset() {
        worker.postMessage({
            type: "reset",
        });
    }

    $effect(() => {
        console.log("emulatorS234ettings", $emulatorSettings);
    });

    emulatorSettings.subscribe((settings) => {
        // worker.postMessage({
        //     type: "updateSettings",
        //     settings,
        // });
        if (settings.loadState) {
            loadState();
            emulatorSettings.update((s) => {
                s.loadState = false;
                return s;
            });
        }
        if (settings.saveState) {
            saveState();
            emulatorSettings.update((s) => {
                s.saveState = false;
                return s;
            });
        }
    });
    
    export function saveState() {
        worker.postMessage({
            type: "saveState",
        });
    }

    export function loadState() {
        const input = document.createElement("input");
        input.style.display = "none";
        input.type = "file";
        input.accept = ".bin";
        input.onchange = (event) => {
            const file = (event.target as HTMLInputElement).files?.[0];
            if (file) {
                const reader = new FileReader();
                reader.onload = () => {
                    worker.postMessage({
                        type: "loadState",
                        state: reader.result,
                    });
                };
                reader.readAsArrayBuffer(file);
            } else {
                alert("Failed to load file");
            }
        };
        input.click();
        input.remove();
    }

    let sidNode: AudioWorkletNode;
    let sidFilterNode: AudioWorkletNode;

    async function audioWorkletSetup() {
        try {
            await audioContext.audioWorklet.addModule("/audio-worklet.js");
            await audioContext.audioWorklet.addModule("/sid-filter-worklet.js");
            sidNode = new AudioWorkletNode(audioContext, "sid-processor", {
                outputChannelCount: [3]
            });
            
            sidFilterNode = new AudioWorkletNode(
                audioContext,
                "sid-filter-processor",
            );
            
            sidNode.port.onmessage = (event) => {
                if (event.data.type === "ready") {
                    console.log("SID audio processor ready");
                }
            };

            sidNode.connect(sidFilterNode);
            sidFilterNode.connect(audioContext.destination);
        } catch (error) {
            console.error("Audio Worklet setup failed:", error);
        }
    }

    function start() {
        audioContext.resume().then(() => {
            audioWorkletSetup().then(() => {
                console.log("Starting sound...");
            });
        });

        setInterval(() => {
            if (sidNode) {
                // sidNode.parameters.get("v1pulseFrequency").value = Math.random() * 1000;
                // sidNode.parameters.get("v1pulseWidth").value = Math.random();
                sidNode.parameters.get("v1On").value = Math.random() > 0.5 ? 1 : 0;

                // sidNode.parameters.get("pulseWidth").value = Math.random();
                // sidNode.port.postMessage({
                //     type: "update",
                //     clockSpeed: clockSpeed,
                // });
            }
        }, 200);
        // console.log("Starting emulator...");
        // worker.postMessage({ type: "start" });
        // addToConsole("Starting emulator...");
    }

    function pause() {
        console.log("Emulation paused");
        addToConsole("Emulation paused");
    }

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

    <canvas
        bind:this={canvas}
        id="canvas"
        style="image-rendering: pixelated;"
        tabindex="1"
    ></canvas>

    <div class="controls">
        <button class="btn" onclick={start}>
            <i class="fas fa-play"></i> Start
        </button>
        <button class="btn btn-secondary" onclick={pause}>
            <i class="fas fa-pause"></i> Pause
        </button>
        <button class="btn btn-secondary" onclick={reset}>
            <i class="fas fa-redo"></i> Reset
        </button>
        <button class="btn btn-secondary" onclick={screenshot}>
            <i class="fas fa-camera"></i> Screenshot
        </button>
        <input
            type="text"
            placeholder="Text Input"
            class="input-field"
            bind:this={textInput}
        />

        <span class="clock-speed">
            Clock Speed: {clockSpeed} Hz
        </span>
    </div>
</div>

<style>
    .clock-speed {
        font-size: 0.9rem;
        color: var(--text);
        margin-top: auto;
        margin-bottom: auto;
        font-weight: 600;
    }
    #canvas {
        width: 100%;
        height: auto;
        background-color: var(--darker);
        border-radius: 10px;
        display: block;
        margin: 0 auto;
        border: 1px solid var(--glass-border);
        aspect-ratio: 1.6;
    }
</style>

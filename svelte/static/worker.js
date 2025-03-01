const Module = {
    onRuntimeInitialized: function () {
        postMessage({ type: "print", text: "WASM Module Loaded" });
    },

    print: function (text) {
        postMessage({ type: "print", text: text });
    },

    printErr: function (text) {
        postMessage({ type: "printErr", text: text });
    }
};

self.framebufferReady = function() {
    var fbPtr = Module.ccall('getFramebuffer', 'number', [], []);
    var length = 320 * 200;
    var fb = new Uint32Array(Module.HEAPU32.buffer, fbPtr, length);
    postMessage({ type: "frame", framebuffer: fb });
};

self.processOtherStuff = function() {
    postMessage({ type: "print", text: "Processing other stuff" });
};

import initModule from "./main.js";
initModule(Module);

onmessage = function (e) {
    let data = e.data;
    if (data.type === "start") {
        Module.ccall("startEmulator", null, [], []);
    } else if (data.type === "test") {
        Module.ccall("test", null, [], []);
    }
};
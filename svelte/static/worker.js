import initModule from "./main.js";


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
    let clockSpeed = Module.ccall('getClockSpeed', 'number', [], []);
    postMessage({ type: "updateInfo", clockSpeed: clockSpeed });
};

function print(text) {
    postMessage({ type: "print", text: text });
}

function printErr(text) {
    postMessage({ type: "printErr", text: text });
}

function writeToMemory(location, byte) {
    if (location < 0) {
        printErr("Invalid address: " + location);
        return;
    }
    if (byte < 0) {
        printErr("Invalid byte: " + byte);
        return;
    }
    location %= 0x100000;
    byte %= 256;
    Module.ccall('writeToMemory', null, ['number', 'number'], [location, byte]);
}

function readFromMemory(location) {
    if (location < 0) {
        printErr("Invalid address: " + location);
        return;
    }
    location %= 0x100000;
    return Module.ccall('readFromMemory', 'number', ['number'], [location]);
}

function inputTextAsBytes(bytes) {
    if (bytes instanceof Uint8Array) {
        bytes = Array.from(bytes);
    }
    
    if (bytes.length < 10) {
        for (let i = 0; i < bytes.length; i++) {
            if (bytes[i] < 0) {
                printErr("Invalid byte: " + bytes[i]);
                return;
            }
            writeToMemory(631 + i, bytes[i] % 256);
        }
        writeToMemory(198, bytes.length);
        return;
    }
    const chunkInterval = setInterval(() => {
        if (readFromMemory(198) !== 0) {
            return;
        }
        if (bytes.length <= 0) {
            clearInterval(chunkInterval);
            return;
        }
        let chunk = bytes.splice(0, 10);
        for (let i = 0; i < chunk.length; i++) {
            if (chunk[i] < 0) {
                printErr("Invalid byte: " + chunk[i]);
                return;
            }
            writeToMemory(631 + i, chunk[i] % 256);
        }
        writeToMemory(198, chunk.length);
    }, 10);

    return;
}

initModule(Module);

onmessage = function (e) {
    let data = e.data;
    if (data.type === "start") {
        Module.ccall("startEmulator", null, [], []);
    } 
    if (data.type === "keyUp") {
        Module.ccall("keyUp", null, ["string"], [data.key]);
    } 
    if (data.type === "keyDown") {
        Module.ccall("keyDown", null, ["string"], [data.key]);
    }
    if (data.type === "inputText") {
        inputTextAsBytes(data.bytes);
    }
    if (data.type === "writeToMemory") {
        if (!data.location || !data.byte) {
            printErr("Invalid data for writeToMemory");
            return;
        }
        writeToMemory(data.location, data.byte);
    }
    if (data.type === "readFromMemory") {
        if (!data.location) {
            printErr("Invalid data for readFromMemory");
            return;
        }
        let value = readFromMemory(data.location);
        postMessage({ type: "readFromMemory", location: data.location, value: value });
    }
};
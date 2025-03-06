#include <bitset>
#include <floppy.hpp>
#include <iostream>

Floppy::Floppy(SerialBus* bus) : SerialDevice(bus) {
    startTime = std::chrono::high_resolution_clock::now();

    state = {true, false, false};
}

SerialPortState Floppy::getIndividualState() {
    return state;
}

void Floppy::shiftBit(bool bit) {
    shiftRegister = (shiftRegister << 1) | bit;
    bitTransfered++;
    if(bitTransfered == 8) {
        byteTransferComplete = true;
        byteTransferInitiated = false;
    }
}

void Floppy::tick() {
    // make sure 1 second has passed before we do anything
    auto currentTime = std::chrono::high_resolution_clock::now();
    if(std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count() < 5) {
        return;
    }
    if(!byteTransferInitiated) {
        state.dataLine = true;
        state.clockLine = false;
    }

    if(!byteTransferInitiated && !byteTransferComplete && !bus->Read(false).clockLine) {
        byteTransferInitiated = true;
        shiftRegister = 0;
        bitTransfered = 0;
        state.dataLine = false;
    }

    if(byteTransferInitiated && !byteTransferComplete && !bus->Read(false).clockLine) {
        shiftBit(bus->Read(false).dataLine);
        std::cout << "Floppy shift register: " << std::bitset<8>(shiftRegister) << std::endl;
    }

    if(byteTransferComplete) {
        state.dataLine = true;
        byteTransferComplete = false;
        byteTransferInitiated = false;
    }
}

// CIA2 write to address: dd0d with data: 01111111
// CIA2 write to address: dd0e with data: 00001000
// CIA2 write to address: dd0f with data: 00001000
// CIA2 write to address: dd03 with data: 00000000
// SID write to address: 18 with data: 0
// CIA2 write to address: dd00 with data: 00000111
// Port A: 00000111
// ATN: 0 CLK: 0 DATA: 0
// CIA2 write to address: dd02 with data: 00111111
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 00010111
// Port A: 00010111
// ATN: 0 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 00010111
// Port A: 00010111
// ATN: 0 CLK: 1 DATA: 0
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00010111
// Port A: 00010111
// ATN: 0 CLK: 1 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00011111
// Port A: 00011111
// ATN: 1 CLK: 1 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00011111
// Port A: 00011111
// ATN: 1 CLK: 1 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00011111
// Port A: 00011111
// ATN: 1 CLK: 1 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00011111
// Port A: 00011111
// ATN: 1 CLK: 1 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000
// CIA2 write to address: dd00 with data: 00001111
// Port A: 00001111
// ATN: 1 CLK: 0 DATA: 0
// Floppy shift register: 00000000
// CIA2 read from address: 0
// Floppy shift register: 00000000

// WASM Module Loaded
// Starting emulator...
// Starting emulator
// CIA2 write to address: dd0d with data: 01111111
// CIA2 write to address: dd0e with data: 00001000
// CIA2 write to address: dd0f with data: 00001000
// CIA2 write to address: dd03 with data: 00000000
// SID write to address: 18 with data: 0
// CIA2 write to address: dd00 with data: 00000111
// Port A: 00000111
// ATN: 0 CLK: 0 DATA: 0
// CIA2 write to address: dd02 with data: 00111111
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 00010111
// Port A: 00010111 // stop of normal startup
// ATN: 0 CLK: 1 DATA: 0
// CIA2 read from address: 0 // load "$",8
// CIA2 write to address: dd00 with data: 01010111
// Port A: 01010111
// ATN: 0 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01010111
// Port A: 01010111
// ATN: 0 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01011111
// Port A: 01011111
// ATN: 1 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01011111
// Port A: 01011111
// ATN: 1 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01011111
// Port A: 01011111
// ATN: 1 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01011111
// Port A: 01011111
// ATN: 1 CLK: 1 DATA: 0
// CIA2 read from address: 0
// CIA2 read from address: 0
// CIA2 read from address: 0
// CIA2 write to address: dd00 with data: 01001111
// Port A: 01001111
// ATN: 1 CLK: 0 DATA: 0
// CIA2 read from address: 0
// CIA2 read from address: 0
// CIA2 read from address: 0
// reapeat forever

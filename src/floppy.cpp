#include <bitset>
#include <floppy.hpp>
#include <iostream>

Floppy::Floppy(SerialBus* bus) : SerialDevice(bus) {
    state = {true, false, false};
    loggingFile.open("floppy.log", std::ios::out | std::ios::trunc);
    if (!loggingFile.is_open()) {
        std::cerr << "Failed to open floppy.log" << std::endl;
    }
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
    std::cout << std::endl;
    SerialPortState busState = bus->Read(false);
    // put the bus state into the logging file
    loggingFile << static_cast<int>(busState.dataLine) << ","
                << static_cast<int>(busState.clockLine) << ","
                << static_cast<int>(busState.atnLine) << std::endl;
    if(!byteTransferInitiated) {
        state.dataLine = true;
        state.clockLine = false;
    }

    bool clockLineSwitchOff = !busState.clockLine && lastClockLine != busState.clockLine;

    if (clockLineSwitchOff) {
        std::cout << "Floppy: clock line changed" << std::endl;
    }

    if(!byteTransferInitiated && !byteTransferComplete && clockLineSwitchOff) {
        std::cout << "Floppy: clock line low" << std::endl;
        byteTransferInitiated = true;
        shiftRegister = 0;
        bitTransfered = 0;
        state.dataLine = false;
    }

    lastClockLine = busState.clockLine;
    std::cout << std::endl;
}

#include <serial_bus.hpp>
#include <iostream>

SerialBus::SerialBus() {
    state = {false, false, true};
    ciaState = {false, false, true};
}

SerialBus::~SerialBus() {
}

void SerialBus::CIAWrite(SerialPortState data) {
    ciaState = data;
    if(data.dataLine) {
        state.dataLine = true;
    }
    if(data.clockLine) {
        state.clockLine = true;
    }
    state.atnLine = data.atnLine;

    for(SerialDevice* device : devices) {
        device->tick();
    }

    printState();
}

SerialPortState SerialBus::Read(bool tick) {
    state = ciaState;
    for(SerialDevice* device : devices) {
        if(tick) {
            // device->tick();
        }
        SerialPortState deviceState = device->getIndividualState();
        if(deviceState.dataLine) {
            state.dataLine = true;
        }
        if(deviceState.clockLine) {
            state.clockLine = true;
        }
    }

    return state;
}

void SerialBus::printState() {
    SerialPortState state = Read(false);
    std::cout << "SerialBus state: "
              << "dataLine: " << state.dataLine
              << ", clockLine: " << state.clockLine
              << ", atnLine: " << state.atnLine
              << std::endl;
}
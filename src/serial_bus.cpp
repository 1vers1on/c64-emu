#include <serial_bus.hpp>

SerialBus::SerialBus() {
    state = {true, true, true};
    ciaState = {true, true, true};
}

SerialBus::~SerialBus() {
}

void SerialBus::CIAWrite(SerialPortState data) {
    ciaState = data;
    if (!data.dataLine) {
        state.dataLine = false;
    }
    if (!data.clockLine) {
        state.clockLine = false;
    }
    state.atnLine = data.atnLine;

    for (SerialDevice* device : devices) {
        device->tick();
    }
}

SerialPortState SerialBus::CIARead() {
    state = ciaState;
    for (SerialDevice* device : devices) {
        device->tick();
        SerialPortState deviceState = device->getIndividualState();
        if (!deviceState.dataLine) {
            state.dataLine = false;
        }
        if (!deviceState.clockLine) {
            state.clockLine = false;
        }
    }

    return state;
}

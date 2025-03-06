#include <serial_bus.hpp>

SerialBus::SerialBus() {
    state = {false, false, true};
    ciaState = {false, false, true};
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

SerialPortState SerialBus::Read(bool tick) {
    state = ciaState;
    for (SerialDevice* device : devices) {
        if (tick) {
            device->tick();
        }
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

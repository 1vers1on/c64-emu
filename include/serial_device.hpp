#pragma once
#include <serial_bus.hpp>

class SerialBus;

class SerialDevice {
public:
    SerialDevice(SerialBus* bus) : bus(bus) {}
    ~SerialDevice();

    virtual SerialPortState getIndividualState() = 0;

    virtual void tick() = 0;
protected:
    SerialBus* bus;
};

#pragma once

#include <serial_device.hpp>
#include <chrono>

class Floppy : public SerialDevice {
public:
    Floppy(SerialBus* bus) : SerialDevice(bus) {}

    SerialPortState getIndividualState() override;

    void tick() override;
private:
    SerialPortState state;
};

#pragma once

#include <cstdint>
#include <serial_device.hpp>
#include <chrono>

class Floppy : public SerialDevice {
public:
    Floppy(SerialBus* bus);

    SerialPortState getIndividualState() override;

    void tick() override;
private:
    bool byteTransferInitiated = false;
    bool byteTransferComplete = false;
    bool dataValid = false;
    uint8_t bitTransfered = 0;

    void shiftBit(bool bit);
    // chrono time point
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

    uint8_t shiftRegister = 0;
    SerialPortState state;
};

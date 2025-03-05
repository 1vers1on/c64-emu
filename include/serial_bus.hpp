#pragma once

struct SerialPortState;

#include <serial_device.hpp>
#include <vector>

struct SerialPortState {
    bool dataLine;
    bool clockLine;
    bool atnLine;
};

class SerialDevice;

class SerialBus {
public:
    SerialBus();
    ~SerialBus();

    void CIAWrite(SerialPortState data);
    SerialPortState CIARead();

    std::vector<SerialDevice*> devices;

private:
    SerialPortState ciaState;
    SerialPortState state;
};

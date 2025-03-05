#include <floppy.hpp>

SerialPortState Floppy::getIndividualState() {
    return state;
}

void Floppy::tick() {
    state.dataLine = true;
    state.clockLine = true;
    state.atnLine = true;
}

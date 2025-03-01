#include <vic.hpp>

#include <iostream>
#include <bitset>

VIC::VIC(Bus* bus) {
    this->bus = bus;
    for (int i = 0; i < 0x2F; i++) {
        registers[i] = 0x00;
    }

    screen.fill(0x000000);
}

VIC::~VIC() {
}

uint8_t VIC::read(uint16_t addr) {
    std::cout << "VIC read from address: " << std::hex << addr << std::dec << std::endl;
    addr &= 0x2F;
    if (addr == 0x11) {
        uint8_t value = registers[0x11];
        value &= 0b11111110;
        value |= (rasterLine >> 8) & 0x01;
        return value;
    }
    if (addr == 0x12) {
        return rasterLine & 0xFF;
    }

    return registers[addr];
}

void VIC::write(uint16_t addr, uint8_t value) {
    addr &= 0xFF;
    if (addr == 0x11) {
        if (value & 0x20) {
            bitmapMode = true;
        } else {
            bitmapMode = false;
        }
    }

    if (addr == 0x16) {
        if (value & 0x10) {
            multiColorMode = true;
        } else {
            multiColorMode = false;
        }
    }
    
    if (addr == 0x18) {
        // get bits 1-3
        charMemOffset = ((value & 0x07) - 1) * 0x800; // subtracting one is the only way this works
        if (value & 0x08) {
            bitmapOffset = 0x2000;
        } else {
            bitmapOffset = 0x0000;
        }

        screenMemoryOffset = ((value & 0xF0) >> 4) * 0x400;
    }
    if (addr == 0x19) { // icr
        registers[0x19] &= ~value; // clear interrupt
    }
    if (addr == 0x20) { // border color
        registers[0x20] = value & 0x0F;
    }
    if (addr == 0x21) { // background color
        registers[0x21] = value & 0x0F;
    }
    registers[addr] = value;
}

void VIC::tick() {
    cycleCounter++;
    rasterCycle++;
    if (rasterCycle >= 63) {
        rasterCycle = 0;
        if (rasterLine < 200) {
            renderScanline();
        }
        rasterLine++;

        if (rasterLine >= 312) {
            rasterLine = 0;
            needsRender = true;
            frameCount++;
            if (framebufferCallback) {
                framebufferCallback(screen);
            }
        }
        handleRasterInterrupts();
    }

    handleDMASteal();
}

void VIC::handleRasterInterrupts() {
    uint16_t valueNeeded = (registers[0x11] << 8) | registers[0x12];
    if (rasterLine == valueNeeded) {
        registers[0x19] |= 0x01;
        checkInterrupts();
    }
}

void VIC::handleDMASteal() {
    bool badLine = (rasterLine & 0x07) == (registers[0x11] & 0x07);
    if (badLine) {
        this->cpu->stallCycles(35);
    }
}

void VIC::renderScanline() {
    int hScroll = registers[0x16] & 0x07;
    int vScroll = registers[0x11] & 0x07;

    int effectiveScanline = (rasterLine + vScroll) % 200;
    int charRow = effectiveScanline / 8;
    int pixelRowWithinChar = effectiveScanline % 8;

    std::vector<uint32_t> lineBuffer(320, 0);

    for (int cellX = 0; cellX < 40; cellX++) {
        uint16_t screenAddr = charRow * 40 + cellX;
        uint8_t charCode = bus->read(screenAddr + bankAddress + screenMemoryOffset);
        uint8_t colorCode = bus->read(0xD800 + charRow * 40 + cellX);

        uint16_t charRomAddr = charCode * 8 + pixelRowWithinChar;
        uint8_t charData = 0;

        if (bankAddress == 0x0000 || bankAddress == 0x8000) {
            if (charMemOffset == 0x1000 || charMemOffset == 0x1800) {
                charData = bus->readCharRom(charRomAddr);

            } else {
                charData = bus->read(charRomAddr + charMemOffset + bankAddress);
            }
        } else if (bitmapMode) {
            charData = bus->read(bitmapOffset + bankAddress + screenAddr); // ?
        } else {
            charData = bus->read(charRomAddr + charMemOffset + bankAddress);
        }

        uint32_t cellPixels[8];
        for (int bit = 0; bit < 8; bit++) {
            bool pixelOn = (charData >> (7 - bit)) & 0x01;
            if (bitmapMode) {
                cellPixels[bit] = pixelOn ? getColor(charCode & 0x0F) : getColor((charCode & 0xF0) >> 4);
            } else {
                cellPixels[bit] = pixelOn ? getColor(colorCode) : getColor(registers[0x21]);
            }
        }

        for (int bit = 0; bit < 8; bit++) {
            int pixelIndex = cellX * 8 + bit;
            if (pixelIndex < 320) {
                lineBuffer[pixelIndex] = cellPixels[bit];
            }
        }
    }

    std::vector<uint32_t> shiftedLine(320, 0);
    for (int i = 0; i < 320; i++) {
        int shiftedIndex = (i + hScroll) % 320;
        shiftedLine[i] = lineBuffer[shiftedIndex];
    }

    int y = effectiveScanline;
    for (int x = 0; x < 320; x++) {
        screen[y * 320 + x] = shiftedLine[x];
    }
}


uint32_t VIC::getColor(uint8_t colorCode) {
    // C64 colors mapped to RGB
    static const uint32_t colors[] = {
        0x000000,
        0xFFFFFF,
        0x880000,
        0xAAFFEE,
        0xCC44CC,
        0x00CC55,
        0x0000AA,
        0xEEEE77,
        0xDD8855,
        0x664400,
        0xFF7777,
        0x333333,
        0x777777,
        0xAAFF66,
        0x0088FF,
        0xBBBBBB
    };

    return colors[colorCode & 0x0F];
}

void VIC::checkInterrupts() {
    if (registers[0x19] & registers[0x1A]) {
        registers[0x19] |= 0x80;
        cpu->triggerIRQ();
    }
}

void VIC::setFramebufferCallback(std::function<void(std::array<uint32_t, 40 * 25 * 8 * 8>&)> callback) {
    framebufferCallback = callback;
}

void VIC::setCpu(CPU* cpu) {
    this->cpu = cpu;
}

#include <algorithm> // for std::fill
#include <array>
#include <bitset>
#include <iostream>
#include <vic.hpp>

VIC::VIC(C64Bus* bus) {
    this->bus = bus;
    // using std::fill to initialize registers
    std::fill(registers, registers + 0x2F, 0x00);
    screen.fill(0x000000);
}

VIC::~VIC() {
}

uint8_t VIC::read(uint16_t addr) {
    // debug output can be toggled if needed
    // std::cout << "VIC read from address: " << std::hex << addr << std::dec << std::endl;
    addr &= 0x2F;
    switch(addr) {
    case 0x11: {
        uint8_t value = registers[0x11] & 0xFE;
        value |= (rasterLine >> 8) & 0x01;
        return value;
    }
    case 0x12:
        return rasterLine & 0xFF;
    default:
        return registers[addr];
    }
}

void VIC::write(uint16_t addr, uint8_t value) {
    addr &= 0xFF;
    switch(addr) {
    case 0x11:
        bitmapMode = (value & 0x20) != 0;
        break;
    case 0x16:
        multiColorMode = (value & 0x10) != 0;
        break;
    case 0x18:
        // bits 1-3: subtract one then multiply to get the proper offset
        charMemOffset = (((value & 0x07) - 1) * 0x800);
        bitmapOffset = (value & 0x08) ? 0x2000 : 0x0000;
        screenMemoryOffset = ((value & 0xF0) >> 4) * 0x400;
        break;
    case 0x19: // icr: clear interrupt bits
        registers[0x19] &= ~value;
        break;
    case 0x20: // border color
        registers[0x20] = value & 0x0F;
        break;
    case 0x21: // background color
        registers[0x21] = value & 0x0F;
        break;
    default:
        break;
    }
    registers[addr] = value;
}

void VIC::tick() {
    cycleCounter++;
    rasterCycle++;
    if(rasterCycle == 63) {
        rasterCycle = 0;
        if(rasterLine < 200) {
            renderScanline();
        }
        rasterLine++;
        if(rasterLine == 312) {
            rasterLine = 0;
            needsRender = true;
            frameCount++;
            if(framebufferCallback) {
                framebufferCallback(screen);
            }
        }
        handleRasterInterrupts();
    }
    handleDMASteal();
}

void VIC::handleRasterInterrupts() {
    uint16_t valueNeeded = (registers[0x11] << 8) | registers[0x12];
    if(rasterLine == valueNeeded) {
        registers[0x19] |= 0x01;
        checkInterrupts();
    }
}

void VIC::handleDMASteal() {
    // bool badLine = false;
    // if(badLine) {
    //     cpu->stallCycles(35);
    // }
}

void VIC::renderScanline() {
    const int hScroll = registers[0x16] & 0x07;
    const int vScroll = registers[0x11] & 0x07;
    const int effectiveScanline = (rasterLine + vScroll) % 200;
    const int charRow = effectiveScanline / 8;
    const int pixelRowWithinChar = effectiveScanline % 8;
    const int baseScreenRow = charRow * 40;
    const uint32_t backgroundColor = getColor(registers[0x21]);
    const int screenOffset = baseScreenRow + bankAddress + screenMemoryOffset;
    const int colorOffset = 0xD800 + baseScreenRow;
    const int y = effectiveScanline;
    const int screenBaseIndex = y * 320;

    const uint32_t multiColor1 = getColor(registers[0x22] & 0x0F);
    const uint32_t multiColor2 = getColor(registers[0x23] & 0x0F);

    if(!multiColorMode) {
        for(int cellX = 0; cellX < 40; cellX++) {
            const uint16_t screenAddr = screenOffset + cellX;
            const uint8_t charCode = bus->read(screenAddr);
            const uint8_t colorCode = bus->read(colorOffset + cellX);
            const uint32_t fgColor = getColor(colorCode & 0x0F);

            uint8_t charData;
            if(bankAddress == 0x0000 || bankAddress == 0x8000) {
                const uint16_t charRomAddr = charCode * 8 + pixelRowWithinChar;
                if(charMemOffset == 0x1000 || charMemOffset == 0x1800) {
                    charData = bus->readCharRom(charRomAddr);
                } else {
                    charData = bus->read(charRomAddr + charMemOffset + bankAddress);
                }
            } else if(bitmapMode) {
                charData = bus->read(bitmapOffset + bankAddress + baseScreenRow + cellX);
            } else {
                const uint16_t charRomAddr = charCode * 8 + pixelRowWithinChar;
                charData = bus->read(charRomAddr + charMemOffset + bankAddress);
            }

            const int screenX = ((cellX * 8) - hScroll + 320) % 320;

            if(bitmapMode) {
                const uint32_t bgColor = getColor((charCode & 0xF0) >> 4);
                const uint32_t fgColor = getColor(charCode & 0x0F);

                for(int bit = 0; bit < 8; bit++) {
                    const bool pixelOn = ((charData >> (7 - bit)) & 0x01) != 0;
                    const int x = (screenX + bit) % 320;
                    screen[screenBaseIndex + x] = pixelOn ? fgColor : bgColor;
                }
            } else {
                for(int bit = 0; bit < 8; bit++) {
                    const bool pixelOn = ((charData >> (7 - bit)) & 0x01) != 0;
                    const int x = (screenX + bit) % 320;
                    screen[screenBaseIndex + x] = pixelOn ? fgColor : backgroundColor;
                }
            }
        }
    } else {
        for(int cellX = 0; cellX < 20; cellX++) {
            const uint16_t screenAddr = screenOffset + cellX;
            const uint8_t charCode = bus->read(screenAddr);
            const uint8_t colorCode = bus->read(colorOffset + cellX);
            const uint32_t color3 = getColor(colorCode & 0x0F);

            uint8_t charData;
            if(bankAddress == 0x0000 || bankAddress == 0x8000) {
                const uint16_t charRomAddr = charCode * 8 + pixelRowWithinChar;
                if(charMemOffset == 0x1000 || charMemOffset == 0x1800) {
                    charData = bus->readCharRom(charRomAddr);
                } else {
                    charData = bus->read(charRomAddr + charMemOffset + bankAddress);
                }
            } else if(bitmapMode) {
                charData = bus->read(bitmapOffset + bankAddress + baseScreenRow + cellX);
            } else {
                const uint16_t charRomAddr = charCode * 8 + pixelRowWithinChar;
                charData = bus->read(charRomAddr + charMemOffset + bankAddress);
            }

            const int screenX = ((cellX * 8) - hScroll + 320) % 320;

            for(int bit = 0; bit < 4; bit++) {
                const uint8_t colorIndex = (charData >> (6 - bit * 2)) & 0x03;
                uint32_t color;

                if(colorIndex == 0) {
                    color = backgroundColor;
                } else if(colorIndex == 1) {
                    color = multiColor1;
                } else if(colorIndex == 2) {
                    color = multiColor2;
                } else {
                    color = color3;
                }

                const int x = (screenX + bit * 2) % 320;
                screen[screenBaseIndex + x] = color;
                screen[screenBaseIndex + ((x + 1) % 320)] = color;
            }
        }
    }
}

uint32_t VIC::getColor(uint8_t colorCode) {
    static const uint32_t colors[] = {
        0x000000, // 0: Black
        0xFFFFFF, // 1: White
        0x894036, // 2: Red
        0x7abfc7, // 3: Cyan
        0x8a46ae, // 4: Purple
        0x68a941, // 5: Green
        0x3e31a2, // 6: Blue
        0xd0dc71, // 7: Yellow
        0x905f25, // 8: Orange
        0x5c4700, // 9: Brown
        0xbb776d, // 10: Light Red
        0x555555, // 11: Dark Grey
        0x808080, // 12: Medium Grey
        0xacea88, // 13: Light Green
        0x7c70da, // 14: Light Blue
        0xababab  // 15: Light Grey
    };
    return colors[colorCode & 0x0F];
}

void VIC::checkInterrupts() {
    if(registers[0x19] & registers[0x1A]) {
        registers[0x19] |= 0x80;
        cpu->triggerIRQ();
    }
}

void VIC::setFramebufferCallback(
    std::function<void(std::array<uint32_t, 40 * 25 * 8 * 8>&)> callback) {
    framebufferCallback = callback;
}

void VIC::setCpu(CPU* cpu) {
    this->cpu = cpu;
}

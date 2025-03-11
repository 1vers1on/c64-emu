#include <algorithm>
#include <bitset>
#include <chrono>
#include <input.hpp>
#include <iostream>
#include <thread>

Input::Input(C64Bus* bus) : bus(bus) {
    for(int i = 0; i < 8; i++) {
        keyMatrix[i] = 0xFF;
    }
}

Input::~Input() {
    stopWriteStringThread();
    if(writeThread.joinable()) {
        writeThread.join();
    }
}

void Input::setKeyPressed(std::string key, bool pressed) {
    auto it = std::find(keys.begin(), keys.end(), key);
    if(it == keys.end()) {
        std::cerr << "Key not found: " << key << std::endl;
        return;
    }

    if(pressed) {
        pressedKeys.insert(key);
    } else {
        pressedKeys.erase(key);
    }
}

void Input::writeString(std::string str) {
    // std::lock_guard<std::mutex> lock(writeMutex);
    // writeStringInternal(str);
    startWriteStringThread(str);
}

void Input::startWriteStringThread(std::string str) {
    stopWriteStringThread();
    stopThread = false;
    writeThread = std::thread(&Input::writeStringInternal, this, str);
}

void Input::stopWriteStringThread() {
    stopThread = true;
    if(writeThread.joinable()) {
        writeThread.join();
    }
}

void Input::writeStringInternal(std::string str) {
    const int chunkSize = 10;
    int length = str.length();
    int index = 0;

    while(index < length && !stopThread) {
        if(bus->read(198) != 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        int chunkLength = std::min(chunkSize, length - index);
        for(int i = 0; i < chunkLength; i++) {
            char c = str[index + i];
            c = std::toupper(c);
            uint8_t petscii = 0;
            switch(c) {
            case '\n':
            case '\r':
                petscii = 0x0D;
                break;
            case 0x0b:
                petscii = 0x14;
                break;
            case ' ':
                petscii = 0x20;
                break;
            case '!':
                petscii = 0x21;
                break;
            case '"':
                petscii = 0x22;
                break;
            case '#':
                petscii = 0x23;
                break;
            case '$':
                petscii = 0x24;
                break;
            case '%':
                petscii = 0x25;
                break;
            case '&':
                petscii = 0x26;
                break;
            case '\'':
                petscii = 0x27;
                break;
            case '(':
                petscii = 0x28;
                break;
            case ')':
                petscii = 0x29;
                break;
            case '*':
                petscii = 0x2A;
                break;
            case '+':
                petscii = 0x2B;
                break;
            case ',':
                petscii = 0x2C;
                break;
            case '-':
                petscii = 0x2D;
                break;
            case '.':
                petscii = 0x2E;
                break;
            case '/':
                petscii = 0x2F;
                break;
            case '\xA3':
                petscii = 0x5c;
                break;
            default:
                petscii = c;
                break;
            }
            bus->write(631 + i, petscii);
        }
        bus->write(198, chunkLength);
        index += chunkLength;
    }
}

uint8_t reverseBits(uint8_t n) {
    n = ((n & 0xF0) >> 4) | ((n & 0x0F) << 4);
    n = ((n & 0xCC) >> 2) | ((n & 0x33) << 2);
    n = ((n & 0xAA) >> 1) | ((n & 0x55) << 1);
    return n;
}

uint8_t Input::readKeyMatrix(uint8_t row) {
    uint8_t output = 0xFF;
    for(int i = 0; i < 8; i++) {
        if((row & (1 << i)) == 0) {
            int keysRow = 7 - i;
            for(int col = 0; col < 8; col++) {
                int keyIndex = keysRow * 8 + col;
                if(pressedKeys.find(keys[keyIndex]) != pressedKeys.end()) {
                    output &= ~(1 << (7 - col));
                }
            }
        }
    }
    return output;
}

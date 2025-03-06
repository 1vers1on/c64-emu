#include <algorithm>
#include <bitset>
#include <input.hpp>
#include <iostream>

Input::Input() {
    for(int i = 0; i < 8; i++) {
        keyMatrix[i] = 0xFF;
    }
}

Input::~Input() {
}

void Input::setKeyPressed(std::string key, bool pressed) {
    // // check if its in keys
    // if (key == "BACKSPACE") {
    //     key = "DELETE";
    // }
    // if (key == "TAB") {
    //     key = "STOP";
    // }
    // if (key == "LEFT SHIFT") {
    //     key = "LSHIFT";
    // }
    // if (key == "RIGHT SHIFT") {
    //     key = "RSHIFT";
    // }
    // if (key == "\\") {
    //     key = "+";
    // }
    // if (key == "'") {
    //     key = ":";
    // }
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

// reverse the order of the bits in a byte
uint8_t reverseBits(uint8_t n) {
    n = ((n & 0xF0) >> 4) | ((n & 0x0F) << 4);
    n = ((n & 0xCC) >> 2) | ((n & 0x33) << 2);
    n = ((n & 0xAA) >> 1) | ((n & 0x55) << 1);
    return n;
}

// the input here is portA ($DC00) usually
// uint8_t Input::readKeyMatrix(uint8_t row) {
//     std::cout << "Reading key matrix for row: " << std::bitset<8>(row) << std::endl;
//     uint8_t output = 0xFF;
//     for (int i = 0; i < 8; i++) {
//         if ((row & (1 << i)) == 0) {
//             for (int j = i * 8; j < (i + 1) * 8; j++) {
//                 if (pressedKeys.find(keys[j]) != pressedKeys.end()) {
//                     output &= ~(1 << (j % 8));
//                     std::cout << "Key pressed: " << keys[j] << " at index: " << j << std::endl;
//                     std::cout << "Output: " << std::bitset<8>(output) << std::endl;

//                 }
//             }
//         }
//     }

//     // return 0;
//     return output;
// }

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

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

const std::vector<std::string> keys = {
    "STOP", "Q", "C=", "SPACE", "2", "CTRL", "<-", "1",
    "/", "^", "=", "RSHIFT", "HOME", ";", "*", "£",
    ",", "@", ":", ".", "-", "L", "P", "+",
    "N", "O", "K", "M", "0", "J", "I", "9",
    "V", "U", "H", "B", "8", "G", "Y", "7",
    "X", "T", "F", "C", "6", "D", "R", "5",
    "LSHIFT", "E", "S", "Z", "4", "A", "W", "3",
    "DOWN", "F5", "F3", "F1", "F7", "RIGHT", "RETURN", "DELETE"
};

// Only supports the keyboard right now
class Input {
public:
    Input();
    ~Input();

    void setKeyPressed(std::string key, bool pressed);

    uint8_t readKeyMatrix(uint8_t row);

private:
    uint8_t keyMatrix[8];
    // std::vector<std::string> pressedKeys;
    std::unordered_set<std::string> pressedKeys;
};

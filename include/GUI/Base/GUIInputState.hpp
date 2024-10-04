#pragma once

#include <cstdint>
#include <limits>
#include <vector>

typedef uint64_t ElementID;

struct GUIInputState
{
    // Max uint64 sentinel for no element active
    ElementID activeElement = std::numeric_limits<uint64_t>::max();

    bool leftMouseJustUp = false;
    bool leftMouseJustDown = false;
    bool leftMousePressed = false;

    int mouseX = 0;
    int mouseY = 0;

    std::vector<unsigned int> charEnterBuffer;
    bool backspaceJustPressed = false;
};
#pragma once

#include <Arduino.h>

class Receiver
{
public:
    void begin();
    bool learn(const String &device, const String &button);
};
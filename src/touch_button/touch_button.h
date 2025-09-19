#ifndef TOUCH_BUTTON_H
#define TOUCH_BUTTON_H

#include <Arduino.h>

class TouchButton
{
private:
    uint8_t touchPin;
    uint32_t onThreshold;
    bool lastState = false;
public:
    TouchButton(uint8_t pin);
    ~TouchButton();

    void init();
    bool isClick();
};

#endif /* TOUCH_BUTTON_H */
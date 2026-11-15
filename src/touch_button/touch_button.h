#ifndef TOUCH_BUTTON_H
#define TOUCH_BUTTON_H

#include <Arduino.h>

class TouchButton
{
private:
    uint8_t touchPin;
    uint32_t onThreshold;
    bool lastState = false;
    bool longReleased = true;
    bool longPressEvent = false;
    uint64_t pressStart;
    uint64_t lastChange;
    uint8_t clickCount;
    uint8_t finishedClicks;
    uint16_t debounceMs = 40;
    uint16_t multiTimeoutMs = 300;
    uint16_t longPressMs = 600;

    void update();
public:
    TouchButton(uint8_t pin);
    ~TouchButton();

    void init();
    bool isClick(uint8_t clicks = 1);
    bool isLongPress();
    // bool isDoubleClick();
    bool isPressed() {return touchRead(touchPin) > onThreshold ? true : false;}
};

#endif /* TOUCH_BUTTON_H */
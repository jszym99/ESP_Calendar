#include "touch_button.h"

TouchButton::TouchButton(uint8_t pin)
{
    touchPin = pin;
}

TouchButton::~TouchButton()
{
}

void TouchButton::init()
{
    onThreshold = touchRead(touchPin) * 1.1;
}

bool TouchButton::isClick()
{
    uint32_t raw = touchRead(touchPin);


    if(raw >= onThreshold && lastState == false)
    {
        lastState = true;
        return lastState;
    }

    lastState = raw >= onThreshold ? true : false;
    return false;
}
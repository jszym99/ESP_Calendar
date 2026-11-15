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

void TouchButton::update()
{
    bool currentState = isPressed();
    uint32_t currentTime = millis();

    if (currentState != lastState && currentTime - lastChange >= debounceMs) {
        lastState = currentState;
        lastChange = currentTime;

        if (currentState) // Rising edge
        {
            pressStart = currentTime;
            longReleased = true;
        }
        else // Falling edge
        {
            if (currentTime - pressStart >= longPressMs) // Long press time elapsed
            {
                // longPressEvent = true;
                clickCount = 0; // Long press rests multi press
            } else {
                clickCount++;
                // lastClickTime = currentTime;
            }
        }
    }

    if (currentState && longReleased && currentTime - pressStart >= longPressMs)
    {
        longPressEvent = true;
        longReleased = false;
        clickCount = 0;
    }

    // Multi click timeout
    if (clickCount > 0 && currentTime - lastChange > multiTimeoutMs)
    {
        // we store the result and reset internally
        finishedClicks = clickCount;
        clickCount = 0;
    }
}

bool TouchButton::isClick(uint8_t clicks)
{
    update();
    if(finishedClicks == clicks)
    {
        finishedClicks = 0;
        return true;
    }

    return false;
}

bool TouchButton::isLongPress()
{
    update();

    if(longPressEvent)
    {
        longPressEvent = false;
        return true;
    }

    return false;
}

// bool TouchButton::isDoubleClick()
// {
//     update();
//     if(millis() - pressStart > 500)
//     {
//         if(clickCount >= 2)
//         {
//             clickCount = 0;
//             return true;
//         }
//     }
//     return false;
// }
#ifndef DISPLAY_PAGE_H
#define DISPLAY_PAGE_H

#include <Arduino.h>
#include "display_item.h"

class DisplayPage
{
protected:
    DisplayItem **items;
    uint8_t itemCount;
    uint8_t smallFont;
    uint8_t largeFont;
    uint16_t primaryColor;
    uint16_t bgColor;
public:
    DisplayPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor);
    DisplayPage();
    ~DisplayPage();

    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startY);
};

class EventsPage : public DisplayPage
{
private:
    /* data */
public:
    EventsPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor);
    EventsPage();
    ~EventsPage();

};




#endif /* DISPLAY_PAGE_H */
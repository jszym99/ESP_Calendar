#ifndef DISPLAY_ITEM_H
#define DISPLAY_ITEM_H
#include <Arduino.h>
#include <OpenFontRender.h>
#include <TFT_eSPI.h>

struct TextElement
{
    std::string text;
    uint8_t fontSize;
};

class DisplayItem
{
protected:
    uint8_t height;
    uint8_t width;
    uint16_t primaryColor;
    uint16_t bgColor;
public:
    DisplayItem(uint16_t fgColor, uint16_t  bgColor);
    ~DisplayItem();

    virtual void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startY);
    uint8_t getHeight() {return height;}
};

class CalendarItem : public DisplayItem
{
private:
    TextElement date;
    TextElement summary;
    TextElement duration;
public:
    CalendarItem(TextElement date, TextElement summary, TextElement duration, uint16_t fgColor, uint16_t  bgColor);
    ~CalendarItem();

    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startY);
};


#endif /* DISPLAY_ITEM_H */
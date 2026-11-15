#ifndef DISPLAY_PAGE_H
#define DISPLAY_PAGE_H

#include <Arduino.h>
#include "widget.h"
#include "cal_manager/cal_manager.h"

constexpr uint8_t maxWidgets = 5;

class DisplayPage
{
protected:
    WidgetBase *widgets[maxWidgets] = {};
    uint8_t widgetCount = 0;
    // uint8_t smallFont;
    // uint8_t largeFont;
    uint16_t primaryColor;
    uint16_t bgColor;
public:
    // DisplayPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor);
    DisplayPage(uint16_t primaryColor, uint16_t bgColor);
    DisplayPage(WidgetBase *(*widgets), uint8_t widgetCount, uint16_t primaryColor, uint16_t bgColor);
    DisplayPage() {}
    ~DisplayPage() {}

    virtual void display(bool fullUpdate = false);
    bool addWidget(WidgetBase *widget);
    void clear() {widgetCount = 0;}
    uint8_t getItemCount() {return widgetCount;}
    void init(TFT_eSPI *tft, OpenFontRender *render);
};

// class EventsPage : public DisplayPage
// {
// private:
//     const CalManager *calendar = nullptr;
// public:
//     // EventsPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor);
//     // EventsPage(WidgetBase *(*widgets), uint8_t widgetCount, uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor);
    
//     EventsPage(const CalManager *cal, uint16_t primaryColor, uint16_t bgColor);
//     EventsPage() {}
//     ~EventsPage() {}

//     void display(TFT_eSPI *tft, OpenFontRender *render);
//     // void addWidget(WidgetBase *widget) {DisplayPage::addWidget(widget);}
//     // void clear() {DisplayPage::clear();}
// };



#endif /* DISPLAY_PAGE_H */
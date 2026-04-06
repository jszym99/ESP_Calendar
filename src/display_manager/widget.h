#ifndef WIDGET_H
#define WIDGET_H
#include <Arduino.h>
#include <OpenFontRender.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <WiFi.h>
#include "cal_manager/cal_manager.h"

#define STR_BUF_SIZE 64u

struct WidgetConfig
{
    uint16_t x;
    uint16_t y;
    uint16_t maxW;
    uint16_t maxH;
    uint32_t fgColor;
    uint32_t bgColor;
};


class WidgetBase
{
protected:
    // uint16_t primaryColor;
    // uint16_t bgColor;
    WidgetConfig config;
    static TFT_eSPI *tft;
    static OpenFontRender *render;
    
public:
    WidgetBase(WidgetConfig _config);
    WidgetBase() {}
    ~WidgetBase() {}

    virtual void draw(bool fullUpdate = false) = 0;
    virtual uint8_t getHeight() = 0;
    virtual uint8_t getWidth() = 0;
    virtual void init(TFT_eSPI *tft, OpenFontRender *render);
};

class TextWidget : public WidgetBase
{
private:
    char text[STR_BUF_SIZE] = "---";
    uint8_t fontSize;
    Align align;
    bool hasChanged = true;
    uint8_t width = 0;
public:
    TextWidget(WidgetConfig _config, const char * text, uint8_t fontSize, Align align = Align::Left);
    TextWidget() {}
    ~TextWidget() {}

    uint8_t getHeight();
    uint8_t getWidth();
    void draw(bool fullUpdate = false);
    void clear();
    uint8_t getFontSize() {return fontSize;}
    // void init(TFT_eSPI *tft, OpenFontRender *render);

    void update(const char *newText);
    bool getHasChanged() {return hasChanged;}
};

class EventWidget : public WidgetBase
{
private:
    TextWidget date;
    TextWidget summary;
    TextWidget duration;
    const CalendarEvent *event;

public:
    EventWidget(WidgetConfig _config, const CalendarEvent *event);
    EventWidget() {}
    ~EventWidget() {}

    void update(const CalendarEvent *newEvent);

    uint8_t getHeight();
    uint8_t getWidth();
    void draw(bool fullUpdate = false);
};

class DynamicEventListWidget : public WidgetBase
{
private:
    EventWidget widgets[5];
    uint8_t widgetCount;
    const CalManager *calendar;
    const CalendarEvent *events[5];
public:
    DynamicEventListWidget(WidgetConfig _config, const CalManager *cal);
    DynamicEventListWidget() {}
    ~DynamicEventListWidget() {}

    void update();

    uint8_t getHeight();
    uint8_t getWidth();
    void draw(bool fullUpdate = false);
};

class HeaderWidget : public WidgetBase
{
private:
    TextWidget text;
    WiFiClass *wifi;
public:
    HeaderWidget(WidgetConfig _config, TextWidget text);
    HeaderWidget(WidgetConfig _config, WiFiClass *wifi);
    HeaderWidget() {}
    ~HeaderWidget() {}

    uint8_t getHeight();
    uint8_t getWidth();
    void draw(bool fullUpdate = false);
};

// class MonthWidget : public WidgetBase
// {
// private:
//     TextWidget days[44];
// public:
//     MonthWidget( uint16_t fgColor, uint16_t bgColor);
//     MonthWidget() {}
//     ~MonthWidget() {}

//     uint8_t getHeight();
//     uint8_t getWidth();
//     void display(uint8_t startX, uint8_t startY, bool fullUpdate = false);
// };

#endif /* WIDGET_H */
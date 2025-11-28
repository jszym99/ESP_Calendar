#ifndef WIDGET_H
#define WIDGET_H
#include <Arduino.h>
#include <OpenFontRender.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <WiFi.h>
#include "cal_manager/cal_manager.h"

#define STR_BUF_SIZE 64u

class WidgetBase
{
protected:
    uint16_t primaryColor;
    uint16_t bgColor;
public:
    WidgetBase(uint16_t fgColor, uint16_t  bgColor);
    WidgetBase() {}
    ~WidgetBase() {}

    virtual void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false) = 0;
    virtual uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render) = 0;
    virtual uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render) = 0;
};

class TextWidget : public WidgetBase
{
private:
    char text[STR_BUF_SIZE] = "---";
    uint8_t fontSize;
    Align align;
    bool hasChanged = true;
public:
    TextWidget(const char * text, uint8_t fontSize, uint16_t textColor, uint16_t bgColor, Align align = Align::Left);
    // TextWidget(const char *(fn) (), uint8_t fontSize, uint16_t textColor, uint16_t bgColor, Align align = Align::Left);
    TextWidget() {}
    ~TextWidget() {}

    uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render);
    uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render);
    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false);
    uint8_t getFontSize() {return fontSize;}

    void update(const char *newText);
    bool getHasChanged() {return hasChanged;}
};

class EventWidget : public WidgetBase
{
private:
    TextWidget date;
    TextWidget summary;
    TextWidget duration;
    // const CalendarEvent *event;
public:
    // EventWidget(TextWidget date, TextWidget summary, TextWidget duration, uint16_t fgColor, uint16_t  bgColor);
    EventWidget(const CalendarEvent *event, uint16_t fgColor, uint16_t  bgColor);
    EventWidget() {}
    ~EventWidget() {}

    void update(const CalendarEvent *newEvent);

    uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render);
    uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render);
    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false);
};

class DynamicEventListWidget : public WidgetBase
{
private:
    EventWidget widgets[5];
    uint8_t widgetCount;
    const CalManager *calendar;
public:
    DynamicEventListWidget(const CalManager *cal, uint16_t fgColor, uint16_t  bgColor);
    DynamicEventListWidget() {}
    ~DynamicEventListWidget() {}

    uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render);
    uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render);
    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false);
};

class HeaderWidget : public WidgetBase
{
private:
    TextWidget text;
    WiFiClass *wifi;
public:
    HeaderWidget(TextWidget text, uint16_t fgColor, uint16_t  bgColor);
    HeaderWidget(WiFiClass *wifi, uint16_t fgColor, uint16_t  bgColor);
    HeaderWidget() {}
    ~HeaderWidget() {}

    uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render);
    uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render);
    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false);
};

class MonthWidget : public WidgetBase
{
private:
    TextWidget days[44];
public:
    MonthWidget( uint16_t fgColor, uint16_t bgColor);
    MonthWidget() {}
    ~MonthWidget() {}

    uint8_t getHeight(TFT_eSPI *tft, OpenFontRender *render);
    uint8_t getWidth(TFT_eSPI *tft, OpenFontRender *render);
    void display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate = false);
};

#endif /* WIDGET_H */
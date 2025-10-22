#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <OpenFontRender.h>
// #include "cal_manager/cal_manager.h"
#include <vector>

constexpr uint8_t maxDisplayBrightness = UINT8_MAX;

struct displayItem
{
    std::string body;
    std::string header;
    std::string footer;
};

class DisplayManager
{
private:
    TFT_eSPI tft = TFT_eSPI();
    OpenFontRender render;
    // CalManager calendar;
    uint8_t smallFont = 16;
    uint8_t largeFont = 24;
    uint16_t primaryColor = TFT_WHITE;
    uint16_t backgroundColor = TFT_BLACK;
    uint8_t brightness = 255;
    std::vector<displayItem> items = {};

    uint8_t drawItem(uint8_t startLine, std::string summary, std::string date = "", std::string duration = "");

public:
    DisplayManager();
    ~DisplayManager();

    void init();

    void displayHeader();

    void addItem(std::string summary, std::string header, std::string footer);
    void display();
    void clearItems();

    void setBrightness(uint8_t value);
    uint8_t getBrightness() {return brightness;}
};


#endif /* DISPLAY_MANAGER_H */
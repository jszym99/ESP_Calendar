#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include <OpenFontRender.h>
#include "cal_manager/cal_manager.h"
#include <vector>

constexpr uint8_t maxDisplayBrightness = UINT8_MAX;

class DisplayManager
{
private:
    TFT_eSPI tft = TFT_eSPI();
    OpenFontRender render;
    // CalManager calendar;
    uint16_t smallFont = 16;
    uint16_t largeFont = 24;
    uint32_t primaryColor = TFT_DARKGREY;
    uint16_t backgroundColor = TFT_BLACK;
    uint8_t brightness = maxDisplayBrightness;

    uint8_t drawItem(uint8_t startLine, String summary, String date = "", String duration = "");

public:
    DisplayManager(/* args */);
    ~DisplayManager();

    void displayHeader();
    void displayEvents(std::vector<eventData> calEvents);

    void setBrightness(uint8_t value);
    uint8_t getBrightness() {return brightness;}
};


#endif /* DISPLAY_MANAGER_H */
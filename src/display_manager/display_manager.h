#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <TimeLib.h>
#include <OpenFontRender.h>
// #include "cal_manager/cal_manager.h"
#include "display_page.h"
#include <vector>

constexpr uint8_t maxDisplayBrightness = UINT8_MAX;
constexpr uint8_t maxPages = 5;

class DisplayManager
{
private:
    static TFT_eSPI tft;
    static OpenFontRender render;
    // static uint8_t smallFont;
    // static uint8_t largeFont;
    static uint16_t primaryColor;
    static uint16_t backgroundColor;
    static uint8_t brightness;
    static DisplayPage *pages[maxPages];
    static uint8_t pageCount;
    static uint8_t currentPage;

public:
    static void init();

    // static void displayHeader();

    static uint8_t addPage(DisplayPage *page);
    static void displayPage(uint8_t index);

    static void display();

    static void nextPage();
    static void previousPage();
    static void setPage(uint8_t pageNum);

    static void setBrightness(uint8_t value);
    static uint8_t getBrightness() {return brightness;}
    static void setPrimaryColor(uint16_t color) { primaryColor = color; }
    static void setBackgroundColor(uint16_t color) { backgroundColor = color; }
    static uint16_t getPrimaryColor() {return primaryColor;}
    static uint16_t getBackgroundColor() {return backgroundColor;}

private:
    // static uint8_t drawItem(uint8_t startLine, std::string summary, std::string date = "", std::string duration = "");
};


#endif /* DISPLAY_MANAGER_H */
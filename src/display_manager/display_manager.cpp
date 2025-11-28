#include "display_manager.h"
#include "fonts/binaryttf.h"
#include "utils/utils.h"


TFT_eSPI DisplayManager::tft = TFT_eSPI();
// uint8_t DisplayManager::smallFont = 16;
// uint8_t DisplayManager::largeFont = 24;
uint16_t DisplayManager::primaryColor = TFT_WHITE;
uint16_t DisplayManager::backgroundColor = TFT_BLACK;
uint8_t DisplayManager::brightness = 255;
OpenFontRender DisplayManager::render;
DisplayPage *DisplayManager::pages[maxPages];
uint8_t DisplayManager::pageCount;
uint8_t DisplayManager::currentPage = 0;

// Constants
constexpr uint8_t xMargin = 4;
constexpr uint8_t xTextMargin = xMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH-2*xMargin-1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;

void DisplayManager::init()
{
    // Init tft and clear display
    tft.init(); 
    tft.fillScreen(backgroundColor);

    pinMode(TFT_BL, OUTPUT); // Set backlight control pin
    setBrightness(brightness); // Set backlight brightness

    // Init text render
    if (render.loadFont(binaryttf, sizeof(binaryttf))) {
        Serial.println("Render initialize error");
        return;
    }
    render.setDrawer(tft); // Set drawer object
}

// void DisplayManager::displayHeader()
// {
//     // Clear header space
//     tft.fillRect(0, 0, TFT_WIDTH, 24, backgroundColor);

//     // Print last update date and time
//     // render.setFontSize(smallFont);
//     // render.setCursor(0,0);
//     render.setFontColor(primaryColor, backgroundColor);
//     render.drawString((datetimeToString(now(), true, false)).c_str(), render.getCursorX(), render.getCursorY(), render.getFontColor(), render.getBackgroundColor());

//     // Draw WiFi symbol
//     tft.drawSmoothArc(240-16, 12, 10, 9, 135, 225, TFT_BLUE, backgroundColor);
//     tft.drawSmoothArc(240-16, 12, 6, 5, 135, 225, TFT_BLUE, backgroundColor);
//     tft.fillSmoothCircle(240-16, 12, 1, TFT_BLUE, backgroundColor);
//     tft.drawLine(0, 20, TFT_WIDTH, 20, primaryColor);
// }

void DisplayManager::setBrightness(uint8_t value)
{
    // Change brightness value
    brightness = value;

    // Update backlight PWM
    float tmp = brightness/16;
    analogWrite(TFT_BL, (UINT8_MAX - (tmp*tmp)));
}

uint8_t DisplayManager::addPage(DisplayPage *page)
{
    if(pageCount < maxPages)
    {
        pages[pageCount] = page;
    }
    return pageCount++;
}

void DisplayManager::displayPage(uint8_t index)
{
    if(index < pageCount)
    {
        pages[index]->display(&tft, &render, true);
    }
}

void DisplayManager::nextPage()
{
    if(pageCount > 1)
    {
        if(currentPage < pageCount - 1)
        {
            currentPage++;
        }
        else
        {
            currentPage = 0;
        }
    }
    displayPage(currentPage);
}

void DisplayManager::previousPage()
{
    if(pageCount > 1)
    {
        if(currentPage > 0)
        {
            currentPage--;
        }
        else
        {
            currentPage = pageCount - 1;
        }
    }
    displayPage(currentPage);
}

void DisplayManager::setPage(uint8_t pageNum)
{
    if(pageNum < pageCount)
    {
        currentPage = pageNum;
    }
}

void DisplayManager::display()
{
    pages[currentPage]->display(&tft, &render);
}
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

    Serial.println("Initializing page");
    for(uint8_t num = 0; num < pageCount; num++)
    {
        pages[num]->init(&tft, &render);
    }
}

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
        tft.fillScreen(backgroundColor);
        pages[index]->display(true);
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
    pages[currentPage]->display();
}
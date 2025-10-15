#include "display_manager.h"
#include "fonts/binaryttf.h"
#include "utils/utils.h"

// Constants
constexpr uint8_t xMargin = 4;
constexpr uint8_t xTextMargin = xMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH-2*xMargin-1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;

DisplayManager::DisplayManager()
{
}

DisplayManager::~DisplayManager()
{
}

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

void DisplayManager::displayHeader()
{
    // Clear header space
    tft.fillRect(0, 0, TFT_WIDTH, 24, backgroundColor);

    // Print last update date and time
    render.setFontSize(smallFont);
    render.setCursor(0,0);
    render.setFontColor(primaryColor, backgroundColor);
    render.drawString((datetimeToString(now(), true, false)).c_str(), render.getCursorX(), render.getCursorY(), render.getFontColor(), render.getBackgroundColor());

    // Draw WiFi symbol
    tft.drawSmoothArc(240-16, 12, 10, 9, 135, 225, TFT_BLUE, backgroundColor);
    tft.drawSmoothArc(240-16, 12, 6, 5, 135, 225, TFT_BLUE, backgroundColor);
    tft.fillSmoothCircle(240-16, 12, 1, TFT_BLUE, backgroundColor);
    tft.drawLine(0, 20, TFT_WIDTH, 20, primaryColor);
}

void DisplayManager::addItem(std::string summary, std::string header, std::string footer)
{
    items.push_back(displayItem{summary, header, footer});
}

void DisplayManager::dsiplay()
{
    uint8_t startY = 24;

    displayHeader();

    for(auto it : items)
    {
        startY = drawItem(startY, it.body, it.header, it.footer);
    }
    // TODO: length limit
}

void DisplayManager::clearItems()
{
    items = {};
    tft.fillRect(0, 24, TFT_WIDTH, TFT_HEIGHT-24, backgroundColor);
}

uint8_t DisplayManager::drawItem(uint8_t startLine, std::string summary, std::string header, std::string footer)
{
    // Constants
    const uint8_t radius = 12;
    const uint8_t lineWidth = 1;

    // Local variables
    uint8_t yOffset = startLine;
    // uint8_t cellHeight = 1.5*largeFont + (true ? 0.25*smallFont : 0) + (!footer.isEmpty() ? smallFont : 0);
    uint8_t cellHeight = TFT_HEIGHT+radius - (yOffset + 0.75*smallFont);

    //Check if item will fit
    if(yOffset + smallFont + 1.5*largeFont + (footer.empty() ? 0 :  0.75*smallFont) >= tftHeight)
    {
        return yOffset;
    }

    // Draw frame
    tft.drawSmoothRoundRect(xMargin, yOffset +  0.75*smallFont, radius, radius+lineWidth-1, cellWidth, cellHeight, primaryColor, backgroundColor);

    // Print header (event date)
    render.setFontColor(primaryColor, backgroundColor);
    if(!header.empty())
    {
        render.setFontSize(smallFont);
        tft.fillRect(xTextMargin+smallFont-xMargin, yOffset+0.25*smallFont+1, render.getTextWidth(header.c_str()) + 2*xMargin, render.getTextHeight(header.c_str()), backgroundColor);
        render.drawString(header.c_str(), xTextMargin+smallFont, yOffset, primaryColor, backgroundColor);
    }
    yOffset += smallFont;
    
    // Print main text (summary)
    render.setFontSize(largeFont);
    // Shorten string if doesn't fit
    if(render.getTextWidth(summary.c_str())+2*xTextMargin >= TFT_WIDTH)
    {
        summary += "...";
    }
    while(render.getTextWidth(summary.c_str())+2*xTextMargin >= TFT_WIDTH)
    {
        summary.erase(summary.length()-4, 1);
    }
    render.drawString(summary.c_str(), xTextMargin, yOffset, primaryColor, backgroundColor);
    yOffset += 1.5*largeFont;

    // Print footer (event duration)
    if(!footer.empty())
    {
        render.setFontSize(smallFont);
        render.rdrawString(footer.c_str(), TFT_WIDTH - (xTextMargin+smallFont), yOffset-smallFont/4, primaryColor, backgroundColor);
        yOffset += smallFont;
    }
    
    // yOffset += 4;

    return yOffset;
}

void DisplayManager::setBrightness(uint8_t value)
{
    // Change brightness value
    brightness = value;

    // Update backlight PWM
    float tmp = brightness/16;
    analogWrite(TFT_BL, (UINT8_MAX - (tmp*tmp)));
}
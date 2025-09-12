#include "display_manager.h"
#include "fonts/binaryttf.h"
#include "utils/utils.h"

// Constants
constexpr uint8_t xMargin = 4;
constexpr uint8_t xTextMargin = xMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH-2*xMargin-1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;

DisplayManager::DisplayManager(/* args */)
{
    // Init tft and clear display
    tft.init(); 
    tft.fillScreen(backgroundColor);

    pinMode(TFT_BL, OUTPUT);
    analogWrite(TFT_BL, (UINT8_MAX - brightness));

    // Init text render
    if (render.loadFont(binaryttf, sizeof(binaryttf))) {
        Serial.println("Render initialize error");
        return;
    }
    render.setDrawer(tft); // Set drawer object
}

DisplayManager::~DisplayManager()
{
}

void DisplayManager::displayHeader()
{
    // Print last update date and time
    render.setFontSize(smallFont);
    render.setCursor(0,0);
    render.setFontColor(primaryColor, backgroundColor);
    render.drawString((DATETIME_STR(now(), true, false)).c_str(), render.getCursorX(), render.getCursorY(), render.getFontColor(), render.getBackgroundColor());

    // Draw WiFi symbol
    tft.drawSmoothArc(240-16, 12, 10, 9, 135, 225, TFT_BLUE, backgroundColor);
    tft.drawSmoothArc(240-16, 12, 6, 5, 135, 225, TFT_BLUE, backgroundColor);
    tft.fillSmoothCircle(240-16, 12, 1, TFT_BLUE, backgroundColor);
    tft.drawLine(0, 20, TFT_WIDTH, 20, primaryColor);
}

void DisplayManager::displayEvents(std::vector<eventData> calEvents)
{
    tft.fillScreen(backgroundColor);

    displayHeader();

    render.setCursor(8,24);
    render.setFontColor(primaryColor, backgroundColor);
    render.setFontSize(largeFont);

    // Display event error
    if(calEvents.empty())
    {
        render.setFontColor(TFT_RED, backgroundColor);
        render.printf("No events found!\n");
        render.seekCursor(0, render.getFontSize());

        return;
    }

    uint8_t startY = 24;
    String startTime;
    String endTime;
    String time;
    String date;
    for(const auto &ev : calEvents)
    {
        // Create date and time strings
        if(ev.startDate/SECS_PER_DAY == ev.endDate/SECS_PER_DAY)
        {
            startTime = (hour(ev.startDate) < 10 ? "0" : "") + String(hour(ev.startDate))
                + ":" + (minute(ev.startDate) < 10 ? "0" : "") + String(minute(ev.startDate));
            endTime = (hour(ev.endDate) < 10 ? "0" : "") + String(hour(ev.endDate))
                + ":" + (minute(ev.endDate) < 10 ? "0" : "") + String(minute(ev.endDate));
        }
        else
        {
            startTime = (day(ev.startDate) < 10 ? "0" : "") + String(day(ev.startDate)) + " " + 
                String(monthShortStr(month(ev.startDate))) + " " + 
                (hour(ev.startDate) < 10 ? "0" : "") + String(hour(ev.startDate)) + ":" + 
                (minute(ev.startDate) < 10 ? "0" : "") + String(minute(ev.startDate));
            endTime = (day(ev.endDate) < 10 ? "0" : "") + String(day(ev.endDate)) + " " + \
                String(monthShortStr(month(ev.endDate))) + " " +
                (hour(ev.endDate) < 10 ? "0" : "") + String(hour(ev.endDate)) + ":" + 
                (minute(ev.endDate) < 10 ? "0" : "") + String(minute(ev.endDate));
        }
        time = (ev.isAllDay ? "All day" : startTime + " - " + endTime);

        // Create date sring and append "today" if needed
        date = (date != DATETIME_STR(ev.startDate, false, true) ? DATETIME_STR(ev.startDate, false, true) : "");
        if(now()/SECS_PER_DAY == ev.startDate/SECS_PER_DAY)
        {
            date += ", Today";
        }

        // Draw the event item
        startY = drawItem(startY, ev.summary, date, time);
    }
    // tft.drawLine(0, 230, TFT_WIDTH, 230, primaryColor);
}

uint8_t DisplayManager::drawItem(uint8_t startLine, String summary, String header, String footer)
{
    // Constants
    const uint8_t radius = 12;
    const uint8_t lineWidth = 1;

    // Local variables
    uint8_t yOffset = startLine;
    // uint8_t cellHeight = 1.5*largeFont + (true ? 0.25*smallFont : 0) + (!footer.isEmpty() ? smallFont : 0);
    uint8_t cellHeight = TFT_HEIGHT+radius - (yOffset + 0.75*smallFont);

    //Check if item will fit
    if(yOffset + smallFont + 1.5*largeFont + (footer.isEmpty() ? 0 :  0.75*smallFont) >= tftHeight)
    {
        return yOffset;
    }

    // Draw frame
    tft.drawSmoothRoundRect(xMargin, yOffset +  0.75*smallFont, radius, radius+lineWidth-1, cellWidth, cellHeight, primaryColor, backgroundColor);

    // Print header (event date)
    render.setFontColor(primaryColor, backgroundColor);
    if(!header.isEmpty())
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
        summary.concat("...");
    }
    while(render.getTextWidth(summary.c_str())+2*xTextMargin >= TFT_WIDTH)
    {
        summary.remove(summary.length()-4, 1);
    }
    render.drawString(summary.c_str(), xTextMargin, yOffset, primaryColor, backgroundColor);
    yOffset += 1.5*largeFont;

    // Print footer (event duration)
    if(!footer.isEmpty())
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
    brightness = value;
    analogWrite(TFT_BL, (UINT8_MAX - brightness));
}
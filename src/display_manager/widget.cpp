#include "widget.h"
#include "utils/utils.h"

uint8_t convertWeekday(uint8_t day)
{
    if(day > 1)
    {
        return day - 2;
    }
    else
    {
        return day + 5;
    }
}

constexpr uint8_t xMargin = 4;
constexpr uint8_t xTextMargin = xMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH-2*xMargin-1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;
constexpr uint8_t radius = 12;
constexpr uint8_t lineWidth = 1;

WidgetBase::WidgetBase(uint16_t fgColor, uint16_t  bgColor) :
primaryColor(fgColor), 
bgColor(bgColor) {}


TextWidget::TextWidget(const char * _text, uint8_t fontSize, uint16_t textColor, uint16_t bgColor, Align align) : 
fontSize(fontSize),
align(align),
WidgetBase(textColor, bgColor)
{
    this->update(_text);
}

uint8_t TextWidget::getHeight(TFT_eSPI *tft, OpenFontRender *render)
{
    return fontSize + 1;
}

uint8_t TextWidget::getWidth(TFT_eSPI *tft, OpenFontRender *render)
{
    FT_BBox bbox = render->calculateBoundingBox(0, 0, fontSize, align, Layout::Horizontal, text);
	return (bbox.xMax - bbox.xMin);
}

void TextWidget::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate)
{
    if(!fullUpdate && !hasChanged)
    {

        return; // No need to redraw
    }

    hasChanged = false;

    render->setFontSize(fontSize);
    render->setAlignment(align);
    render->setFontColor(primaryColor);
    render->setBackgroundColor(bgColor);

    char buf[64];
    strcpy(buf, text);

    // Shorten string if it doesn't fit
    if(render->getTextWidth(text)+2*xTextMargin >= TFT_WIDTH)
    {
        uint32_t dotsWidth{render->getTextWidth("...")};

        while(render->getTextWidth(buf)+2*xTextMargin + dotsWidth >= TFT_WIDTH)
        {
            buf[strlen(buf)-1] = '\0';
        }

        strcat(buf, "...");
    }

    uint8_t bgWidth = render->getTextWidth(buf) + 2 * xMargin;
    uint8_t bgHeight = fontSize + 1;
    uint8_t yOffset = fontSize/3 - 1;

    switch (align)
    {
    case Align::Left:
        tft->fillRect(startX - xMargin, startY + yOffset, bgWidth, bgHeight, bgColor);
        break;
    case Align::Center:
        tft->fillRect(startX - xMargin - bgWidth/2, startY + yOffset, bgWidth, bgHeight, bgColor);
        break;
    case Align::Right:
        tft->fillRect(startX - xMargin - bgWidth, startY + yOffset, bgWidth, bgHeight, bgColor);
        break;
    default:
        break;
    }

    // FT_BBox bbox = render->calculateBoundingBox(0, 0, fontSize, align, Layout::Horizontal, text);
    // tft->drawRect(startX, startY + yOffset, bbox.xMax, fontSize + 1, TFT_RED);


    render->drawString(buf, startX, startY, primaryColor, bgColor);
}

void TextWidget::update(const char *newText)
{
    if(strcmp(text, newText))
    {
        hasChanged = true;
        strncpy(text, newText, STR_BUF_SIZE);
    }
}

EventWidget::EventWidget(const CalendarEvent *event, uint16_t fgColor, uint16_t  bgColor) :
WidgetBase(fgColor, bgColor)
{
    if(event != nullptr)
    {
        date = TextWidget(event->getDateStr(), 16, fgColor, bgColor);
        summary = TextWidget(event->getSummary(), 24, fgColor, bgColor);
        duration = TextWidget(event->getDurationStr(), 16, fgColor, bgColor, Align::Right);
    }
    else
    {
        date = TextWidget("", 16, fgColor, bgColor);
        summary = TextWidget("", 24, fgColor, bgColor);
        duration = TextWidget("", 16, fgColor, bgColor, Align::Right);
    }
}

uint8_t EventWidget::getHeight(TFT_eSPI *tft, OpenFontRender *render)
{
    return date.getHeight(tft, render) + 1.5 * summary.getHeight(tft, render) + duration.getHeight(tft, render);
}

uint8_t EventWidget::getWidth(TFT_eSPI *tft, OpenFontRender *render)
{
    return TFT_WIDTH;
}

void EventWidget::update(const CalendarEvent *newEvent)
{
    date.update(newEvent->getDateStr());
    summary.update(newEvent->getSummary());
    duration.update(newEvent->getDurationStr());
}

void EventWidget::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate)
{
    
    uint8_t cellHeight = TFT_HEIGHT + radius - (startY + 0.75 * date.getFontSize());
    
    // Clear previous text
    if(date.getHasChanged())
    {
        tft->fillRect(startX + xTextMargin, startY + date.getFontSize()/3 - 1, TFT_WIDTH - 2 * xTextMargin, date.getHeight(tft, render), bgColor);
    }
    
    if(fullUpdate)
    {
        tft->fillRect(startX + xTextMargin, startY, getWidth(tft, render) - 2 * xTextMargin, getHeight(tft, render), bgColor);
    }

    if(date.getHasChanged() || fullUpdate)
    {
        // Draw frame
        tft->drawSmoothRoundRect(xMargin, startY + 0.75 * date.getFontSize(), radius, radius+lineWidth-1, cellWidth, cellHeight, primaryColor, bgColor);
    }
    // tft->fillRect(xTextMargin + date.fontSize - xMargin, startY + 0.25 * date.fontSize + 1, render->getTextWidth(date.text.c_str()) + 2 * xMargin, render->getTextHeight(date.text.c_str()), bgColor);

    // render->setBackgroundFillMethod(BgFillMethod::Block);
    // render->setFontSize(date.fontSize);
    // render->drawString(date.text.c_str(), xTextMargin + date.fontSize, startY, primaryColor, bgColor);

    date.display(tft, render, xTextMargin + date.getFontSize(), startY, fullUpdate);

    startY += date.getFontSize();

    // Clear previous text
    // if(summary.getHasChanged())
    // {
    //     // tft->fillRect(startX + xTextMargin, startY, getWidth(tft, render) - 2 * xTextMargin, summary.getHeight(tft, render), bgColor);
    //     tft->drawRect(startX + xTextMargin, startY + (summary.getHeight(tft, render)/3), getWidth(tft, render) - 2 * xTextMargin, summary.getHeight(tft, render) - 1, primaryColor);
    // }

    summary.display(tft, render, xTextMargin, startY, fullUpdate);

    // render->setFontSize(summary.fontSize);
    // render->drawString(summary.text.c_str(), xTextMargin, startY, primaryColor, bgColor);

    startY += 1.25 * summary.getFontSize();


    // // Clear previous text
    // if(duration.getHasChanged())
    // {
    //     // tft->fillRect(startX + xTextMargin, startY + 1, getWidth(tft, render) - 2 * xTextMargin, duration.getHeight(tft, render), bgColor);
    //     tft->drawRect(startX + xTextMargin, startY + (duration.getHeight(tft, render)/4), getWidth(tft, render) - 2 * xTextMargin, duration.getHeight(tft, render) - 1, primaryColor);
    // }

    duration.display(tft, render, TFT_WIDTH - (xTextMargin + duration.getFontSize()), startY, fullUpdate);


    // render->setFontSize(duration.fontSize);
    // render->rdrawString(duration.text.c_str(), TFT_WIDTH - (xTextMargin+duration.fontSize), startY, primaryColor, bgColor);
}

DynamicEventListWidget::DynamicEventListWidget(const CalManager *cal, uint16_t fgColor, uint16_t  bgColor) :
calendar(cal), WidgetBase(fgColor, bgColor)
{
    if(calendar != nullptr)
    {
        const std::vector<CalendarEvent> events = calendar->getEvents();

        for(uint8_t num = 0; num < eventCount; num++)
        {
            widgets[num] = EventWidget(nullptr, TFT_WHITE, TFT_BLACK);
        }
    }
}

uint8_t DynamicEventListWidget::getHeight(TFT_eSPI *tft, OpenFontRender *render)
{
    widgetCount = calendar->getEventCount();
    return widgetCount * widgets[0].getHeight(tft, render);
}

uint8_t DynamicEventListWidget::getWidth(TFT_eSPI *tft, OpenFontRender *render)
{
    return TFT_WIDTH;
}

void DynamicEventListWidget::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate)
{
    uint8_t endY;
    
    if(calendar != nullptr)
    {
        if(widgetCount != calendar->getEventCount()) // Full redraw on list length change
        {
            fullUpdate = true;
        }

        if(fullUpdate)
        {
            tft->fillRect(startX, startY, getWidth(tft, render), getHeight(tft, render), bgColor); // Erase old list
        }

        widgetCount = calendar->getEventCount();
        const std::vector<CalendarEvent> events = calendar->getEvents();

        for(uint8_t num = 0; num < eventCount; num++)
        {
            widgets[num].update(&events[num]);
        }

        for(uint8_t num = 0; num < widgetCount; num++)
        {
            endY = startY + widgets[num].getHeight(tft, render);
    
            if(endY < TFT_WIDTH && endY > startY) // Check if will fit
            {
                widgets[num].display(tft, render, 0, startY, fullUpdate);
                startY = endY;
            }
        }
    }

}

HeaderWidget::HeaderWidget(TextWidget text, uint16_t fgColor, uint16_t  bgColor) :
text(text),
WidgetBase(fgColor, bgColor) {}

HeaderWidget::HeaderWidget(WiFiClass *wifi, uint16_t fgColor, uint16_t  bgColor) :
wifi(wifi),
WidgetBase(fgColor, bgColor)
{
    text = TextWidget((datetimeToString(now(), true, false, true)).c_str(), 16, fgColor, bgColor);
}

uint8_t HeaderWidget::getHeight(TFT_eSPI *tft, OpenFontRender *render)
{
    // return text.getHeight(tft, render);
    return 24;
}

uint8_t HeaderWidget::getWidth(TFT_eSPI *tft, OpenFontRender *render)
{
    return TFT_WIDTH;
}

void HeaderWidget::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate)
{
    text.update((datetimeToString(now(), true, false, true)).c_str());

    if(fullUpdate || text.getHasChanged())
    {
        tft->fillRect(startX + xTextMargin, startY + text.getFontSize()/3 - 1, TFT_WIDTH - 2 * xTextMargin, text.getHeight(tft, render), bgColor);
    }

    text.display(tft, render, xTextMargin, startY, fullUpdate);

    if(fullUpdate)
    {
        tft->fillRect(TFT_WIDTH-32, 0, 32, 32, bgColor);
    }

    if(wifi->isConnected())
    {
        // Draw WiFi symbol
        tft->drawSmoothArc(TFT_WIDTH-16, 12, 10, 9, 135, 225, TFT_BLUE, bgColor);
        tft->drawSmoothArc(TFT_WIDTH-16, 12, 6, 5, 135, 225, TFT_BLUE, bgColor);
        tft->fillSmoothCircle(TFT_WIDTH-16, 12, 1, TFT_BLUE, bgColor);

        tft->drawLine(0, 20, TFT_WIDTH, 20, primaryColor);
    }
}

MonthWidget::MonthWidget(uint16_t fgColor, uint16_t bgColor) :
WidgetBase(fgColor, bgColor) {} 

uint8_t MonthWidget::getHeight(TFT_eSPI *tft, OpenFontRender *render)
{
    return 7*24;
}

uint8_t MonthWidget::getWidth(TFT_eSPI *tft, OpenFontRender *render)
{
    return TFT_WIDTH;
}

void MonthWidget::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startX, uint8_t startY, bool fullUpdate)
{
    time_t time = now();
    const uint8_t colW = 29;
    const uint8_t rowH = 29;
    const char dayLetter[] = "MTWTFSS\0";

    const time_t firstOfMonth = makeTime({.Day = 1, .Month = month(time), .Year = CalendarYrToTm(year(time))});
    const time_t firstOfYear = makeTime({.Day = 1, .Month = 1, .Year = CalendarYrToTm(year(time))});

    uint8_t weekOfYear = (firstOfMonth - firstOfYear)/SECS_PER_WEEK + 1;

    uint8_t dayNum = day(firstOfMonth);

    render->setFontSize(18);
    render->setAlignment(Align::Center);
    render->setFontColor(primaryColor);
    render->setBackgroundColor(bgColor);
    
    for(uint8_t col = 1; col < 9; col++)
    {
        static char buf[2] = "\0";
        buf[0] = dayLetter[col-1];
        render->drawString(buf, startX + xMargin + col * colW + colW/2+1, startY, primaryColor, bgColor);
        tft->drawLine(startX + xMargin + col * colW, startY + rowH,
                      startX + xMargin + col * colW, startY + 7 * rowH, primaryColor);
    }

    for(uint8_t row = 1; row < 8; row++)
    {
        tft->drawLine(startX + xMargin + colW, startY + row * rowH,
                      startX + xMargin + 8 * colW, startY + row * rowH, primaryColor);
    }

    uint8_t x = startX + xMargin + 1.5 * colW;
    uint8_t y = startY + rowH + (rowH-render->getFontSize())/2;
    static char buf[4] = "\0";

    time_t currentTime = firstOfMonth - convertWeekday(weekday(firstOfMonth)) * SECS_PER_DAY;
    for(uint8_t row = 0; row < 6; row++)
    {
        itoa(weekOfYear, buf, 10);
        render->drawString(buf, startX + xMargin + colW/2+1, y + row * rowH , primaryColor, bgColor);
        for(uint8_t col = 0; col < 7; col++)
        {
            if(month(currentTime) != month(time))
            {
                render->setFontColor(TFT_DARKGREY);
                render->setBackgroundColor(bgColor);
            }
            else
            {
                if(day(currentTime) == day(time))
                {
                    render->setFontColor(TFT_BLACK);
                    render->setBackgroundColor(TFT_DARKGREY);
                    tft->fillSmoothRoundRect(startX + xMargin + col * colW + colW + 2, startY + row * rowH + rowH + 2, colW - 3, rowH - 3, 4, TFT_DARKGREY, bgColor);
                    // tft->fillCircle(x + col * colW + 1, startY + rowH + row * rowH + rowH/2 + 1, rowH/2-3, TFT_SKYBLUE);
                }
                else
                {
                    render->setFontColor(primaryColor);
                    render->setBackgroundColor(bgColor);
                }
            }
            memset(buf, 0, sizeof(buf)/sizeof(buf[0]));
            // dayNum = day(firstOfYear + weekOfYear * SECS_PER_WEEK);
            itoa(day(currentTime), buf, 10);
            render->drawString(buf, x + col * colW, y + row * rowH , primaryColor, bgColor);
            currentTime += SECS_PER_DAY;
        }
        weekOfYear++;
    }
}
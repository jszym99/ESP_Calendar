#include "widget.h"
#include "utils/utils.h"

// Widget layout constants
constexpr uint8_t verticalMargin = 4;
constexpr uint8_t verticalTextMargin = verticalMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH - 2 * verticalMargin - 1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;
constexpr uint8_t radius = 12;
constexpr uint8_t lineWidth = 1;
constexpr uint8_t smallFont = 16;
constexpr uint8_t largeFont = 24;

TFT_eSPI *WidgetBase::tft = nullptr;
OpenFontRender *WidgetBase::render = nullptr;

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

WidgetBase::WidgetBase(WidgetConfig _config) :
config(_config) {}

void WidgetBase::init(TFT_eSPI *_tft, OpenFontRender *_render)
{
    tft = _tft;
    render = _render;
}

TextWidget::TextWidget(WidgetConfig _config, const char * _text, uint8_t fontSize, Align align) : 
fontSize(fontSize),
align(align),
WidgetBase(_config)
{
    strncpy(text, _text, STR_BUF_SIZE);
    hasChanged = true;

    // Make sure text fits
    if(fontSize > config.maxH)
    {
        fontSize = config.maxH;
    }
}

uint8_t TextWidget::getHeight()
{
    return fontSize + 1;
}

uint8_t TextWidget::getWidth()
{
    if(text != nullptr)
    {
        FT_BBox bbox = render->calculateBoundingBox(0, 0, fontSize, align, Layout::Horizontal, text);
        return (bbox.xMax - bbox.xMin) < config.maxW ? (bbox.xMax - bbox.xMin) : config.maxW;
    }
    else
    {
        return 0;
    }

}

void TextWidget::draw(bool fullUpdate)
{
    if(!fullUpdate && !hasChanged)
    {

        return; // No need to redraw
    }

    hasChanged = false;

    render->setFontSize(fontSize);
    render->setAlignment(align);
    render->setFontColor(config.fgColor);
    render->setBackgroundColor(config.bgColor);

    char buf[64];
    strcpy(buf, text);

    // Shorten string if it doesn't fit
    if(render->getTextWidth(text) >= config.maxW)
    {
        uint32_t dotsWidth{render->getTextWidth("...")};

        // Remove last character until the string fits
        while(render->getTextWidth(buf) + dotsWidth >= config.maxW)
        {
            buf[strlen(buf)-1] = '\0';
        }

        // End string with dots
        strcat(buf, "...");
    }

    uint8_t textWidth = render->getTextWidth(buf);
    uint8_t bgWidth = textWidth + 2 * verticalMargin;
    uint8_t bgHeight = fontSize + 1;
    uint8_t yOffset = fontSize/3 - 1;

    width = textWidth;

    // Draw background for the text
    switch (align)
    {
    case Align::Left:
        tft->fillRect(config.x - verticalMargin, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    case Align::Center:
        tft->fillRect(config.x - bgWidth/2, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    case Align::Right:
        tft->fillRect(config.x + verticalMargin - bgWidth, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    default:
        break;
    }

    render->drawString(buf, config.x, config.y, config.fgColor, config.bgColor);
}

void TextWidget::clear()
{
    uint8_t bgWidth = width + 2 * verticalMargin;
    uint8_t bgHeight = fontSize + 1;
    uint8_t yOffset = fontSize/3 - 1;

    // Draw background for the text
    switch (align)
    {
    case Align::Left:
        tft->fillRect(config.x, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    case Align::Center:
        tft->fillRect(config.x - bgWidth/2, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    case Align::Right:
        tft->fillRect(config.x + verticalMargin - bgWidth, config.y + yOffset, bgWidth, bgHeight, config.bgColor);
        break;
    default:
        break;
    }
}

void TextWidget::update(const char *newText)
{
    if(strcmp(text, newText))
    {
        hasChanged = true;
        strncpy(text, newText, STR_BUF_SIZE);
    }
}

EventWidget::EventWidget(WidgetConfig _config, const CalendarEvent *_event) :
event(_event),
WidgetBase(_config)
{
    WidgetConfig dateConfig = 
    {
        config.x + verticalTextMargin + smallFont,
        config.y,
        config.maxW - 2 * (config.x + verticalTextMargin + smallFont),
        smallFont,
        config.fgColor,
        config.bgColor,
    };

    WidgetConfig summaryConfig = 
    {
        config.x + verticalTextMargin,
        config.y + smallFont * 0.75,
        config.maxW - 2 * (config.x + verticalTextMargin),
        largeFont,
        config.fgColor,
        config.bgColor,
    };

    WidgetConfig durationConfig = 
    {
        TFT_WIDTH - (verticalTextMargin + smallFont),
        config.y + smallFont + 1.25 * largeFont,
        config.maxW - 2 * (config.x + verticalTextMargin + smallFont),
        smallFont,
        config.fgColor,
        config.bgColor,
    };

    if(event != nullptr)
    {
        date = TextWidget(dateConfig, _event->getDateStr(), smallFont);
        summary = TextWidget(summaryConfig, _event->getSummary(), largeFont);
        duration = TextWidget(durationConfig, _event->getDurationStr(), smallFont, Align::Right);
    }
    else
    {
        date = TextWidget(dateConfig, "", smallFont);
        summary = TextWidget(summaryConfig, "", largeFont);
        duration = TextWidget(durationConfig, "", smallFont, Align::Right);
    }
}

uint8_t EventWidget::getHeight()
{
    return date.getHeight() + 1.25 * summary.getHeight() + duration.getHeight();
}

uint8_t EventWidget::getWidth()
{
    return TFT_WIDTH;
}

void EventWidget::update(const CalendarEvent *newEvent)
{
    date.update(newEvent->getDateStr());
    summary.update(newEvent->getSummary());
    duration.update(newEvent->getDurationStr());
}

void EventWidget::draw(bool fullUpdate)
{
    
    uint8_t cellHeight = TFT_HEIGHT + radius - (config.y + 0.75 * date.getFontSize());
    // uint8_t startX = config.x;
    // uint8_t startY = config.y;
    
    // Clear previous text
    if(date.getHasChanged())
    {
        // tft->fillRect(startX + verticalTextMargin, startY + date.getFontSize()/3 - 1, TFT_WIDTH - 2 * verticalTextMargin, date.getHeight(), config.bgColor);
        date.clear();
    }
    
    if(fullUpdate)
    {
        tft->fillRect(config.x + verticalTextMargin, config.y, getWidth() - 2 * verticalTextMargin, getHeight(), config.bgColor);
    }

    if(date.getHasChanged() || fullUpdate)
    {
        // Draw frame
        // tft->drawSmoothRoundRect(verticalMargin, startY + 0.75 * date.getFontSize(), radius, radius+lineWidth-1, cellWidth, cellHeight, config.fgColor, config.bgColor);
        tft->drawSmoothRoundRect(config.x + verticalMargin, config.y + 0.75 * date.getFontSize(), radius, radius+lineWidth-1, cellWidth, cellHeight, config.fgColor, config.bgColor);
        
    }

    date.draw(fullUpdate);

    // startY += date.getFontSize();

    // Clear previous text
    if(summary.getHasChanged())
    {
        // tft->fillRect(startX + verticalTextMargin, startY + summary.getFontSize()/3 - 1, TFT_WIDTH - 2 * verticalTextMargin, summary.getHeight(), config.bgColor);
        summary.clear();    
    }

    summary.draw(fullUpdate);

    // startY += 1.25 * summary.getFontSize();


    // Clear previous text
    if(duration.getHasChanged())
    {
        // tft->fillRect(startX + verticalTextMargin, startY + duration.getFontSize()/3 - 1, TFT_WIDTH - 2 * verticalTextMargin, duration.getHeight(), config.bgColor);
        duration.clear();
    }

    duration.draw(fullUpdate);
}

DynamicEventListWidget::DynamicEventListWidget(WidgetConfig _config, const CalManager *cal) :
calendar(cal),
WidgetBase(_config)
{
    if(calendar != nullptr)
    {
        // memcpy(events, calendar->getEventsPtr(), calendar->getEventCount()*sizeof(*calendar->getEventsPtr()));
        for(uint8_t num = 0; num < maxEventCount; num++)
        {
            events[num] = nullptr;//&calendar->getEventsPtr()[num];
            uint16_t height = 2 * smallFont + 1.5 * largeFont;
            uint16_t y = _config.y + num * height;
            widgets[num] = EventWidget(WidgetConfig{0, y, config.maxW, height, config.fgColor, config.bgColor}, events[num]);
        }
    }
}

uint8_t DynamicEventListWidget::getHeight()
{
    widgetCount = calendar->getEventCount();
    return widgetCount * widgets[0].getHeight();
}

uint8_t DynamicEventListWidget::getWidth()
{
    return TFT_WIDTH;
}

void DynamicEventListWidget::update()
{
        // const std::vector<CalendarEvent> events = calendar->getEvents();

        const CalendarEvent *eventsPtr = calendar->getEventsPtr();
        uint8_t eventCount = calendar->getEventCount();

        for(uint8_t num = 0; num < eventCount; num++)
        {
            widgets[num].update(&eventsPtr[num]);
        }
}

void DynamicEventListWidget::draw(bool fullUpdate)
{
    uint8_t endY;
    uint8_t startY = config.y;
    
    if(calendar != nullptr)
    {
        if(widgetCount != calendar->getEventCount()) // Full redraw on list length change
        {
            fullUpdate = true;
        }

        if(fullUpdate)
        {
            tft->fillRect(config.x, config.y, getWidth(), getHeight(), config.bgColor); // Erase old list
        }

        widgetCount = calendar->getEventCount();
        
        for(uint8_t num = 0; num < widgetCount; num++)
        {
            // events[num] = &calendar->getEventsPtr()[num];
            endY = startY + widgets[num].getHeight();
    
            if(endY < config.maxH && endY > startY) // Check if will fit
            {
                widgets[num].draw(fullUpdate);
                startY = endY;
            }
        }

        // Draw bottom of last item
        tft->drawLine(config.x + verticalMargin, endY, config.x + verticalMargin, TFT_HEIGHT, config.bgColor);
        tft->drawLine(config.x + (config.maxW - 1) - verticalMargin, endY, config.x + (config.maxW - 1) - verticalMargin, TFT_HEIGHT, config.bgColor);
        tft->drawArc(config.x + verticalMargin + radius, endY, radius, radius, 0, 90, config.fgColor, config.bgColor);
        tft->drawArc(config.x + (config.maxW - 1) - (verticalMargin + radius), endY, radius, radius, 270, 360, config.fgColor, config.bgColor);
        tft->drawLine(config.x + verticalMargin + radius, endY + radius, config.x + (config.maxW - 1) - (verticalMargin + radius), endY + radius, config.fgColor);
    }

}

HeaderWidget::HeaderWidget(WidgetConfig _config, TextWidget text) :
text(text),
WidgetBase(_config) {}

HeaderWidget::HeaderWidget(WidgetConfig _config, WiFiClass *wifi) :
wifi(wifi),
WidgetBase(_config)
{
    text = TextWidget(WidgetConfig{0,0, config.maxW, smallFont, config.fgColor, config.bgColor}, "", smallFont);
    text.update(datetimeStr(now()));
}

uint8_t HeaderWidget::getHeight()
{
    // return text.getHeight();
    return 24;
}

uint8_t HeaderWidget::getWidth()
{
    return TFT_WIDTH;
}

void HeaderWidget::draw(bool fullUpdate)
{
    text.update(datetimeStr(now()));

    if(fullUpdate || text.getHasChanged())
    {
        // tft->fillRect(config.x + verticalTextMargin, config.y + text.getFontSize()/3 - 1, TFT_WIDTH - 2 * verticalTextMargin, text.getHeight(), config.bgColor);
        text.clear();
    }

    text.draw(fullUpdate);

    if(fullUpdate)
    {
        tft->fillRect(TFT_WIDTH-32, 0, 32, 32, config.bgColor);
    }

    if(wifi->isConnected())
    {
        // Draw WiFi symbol
        tft->drawSmoothArc(config.x + (config.maxW - 1) - 16, 12, 10, 9, 135, 225, TFT_BLUE, config.bgColor);
        tft->drawSmoothArc(config.x + (config.maxW - 1) - 16, 12, 6, 5, 135, 225, TFT_BLUE, config.bgColor);
        tft->fillSmoothCircle(config.x + (config.maxW - 1) - 16, 12, 1, TFT_BLUE, config.bgColor);
    }
    else
    {
        tft->fillRect(config.x + (config.maxW - 1) - 22, config.y, 24, 24, config.bgColor);
    }

    tft->drawLine(config.x, 20, config.x + (config.maxW - 1), 20, config.fgColor);
}

// MonthWidget::MonthWidget(uint16_t fgColor, uint16_t bgColor) :
// WidgetBase(fgColor, bgColor) {} 

// uint8_t MonthWidget::getHeight()
// {
//     return 7*24;
// }

// uint8_t MonthWidget::getWidth()
// {
//     return TFT_WIDTH;
// }

// void MonthWidget::display(uint8_t startX, uint8_t startY, bool fullUpdate)
// {
//     if(fullUpdate)
//     {
//         tft->fillRect(startX, startY, getWidth(), getHeight(), bgColor); // Erase old widget
//     }

//     time_t time = now();
//     const uint8_t colW = 29;
//     const uint8_t rowH = 29;
//     const char dayLetter[] = "MTWTFSS\0";

//     const time_t firstOfMonth = makeTime({.Day = 1, .Month = month(time), .Year = CalendarYrToTm(year(time))});
//     const time_t firstOfYear = makeTime({.Day = 1, .Month = 1, .Year = CalendarYrToTm(year(time))});

//     uint8_t weekOfYear = (firstOfMonth - firstOfYear)/SECS_PER_WEEK + 1;

//     uint8_t dayNum = day(firstOfMonth);

//     render->setFontSize(18);
//     render->setAlignment(Align::Center);
//     render->setFontColor(primaryColor);
//     render->setBackgroundColor(bgColor);
    
//     for(uint8_t col = 1; col < 9; col++)
//     {
//         static char buf[2] = "\0";
//         buf[0] = dayLetter[col-1];
//         render->drawString(buf, startX + xMargin + col * colW + colW/2+1, startY, primaryColor, bgColor);
//         tft->drawLine(startX + xMargin + col * colW, startY + rowH,
//                       startX + xMargin + col * colW, startY + 7 * rowH, primaryColor);
//     }

//     for(uint8_t row = 1; row < 8; row++)
//     {
//         tft->drawLine(startX + xMargin + colW, startY + row * rowH,
//                       startX + xMargin + 8 * colW, startY + row * rowH, primaryColor);
//     }

//     uint8_t x = startX + xMargin + 1.5 * colW;
//     uint8_t y = startY + rowH + (rowH-render->getFontSize())/2;
//     static char buf[4] = "\0";

//     time_t currentTime = firstOfMonth - convertWeekday(weekday(firstOfMonth)) * SECS_PER_DAY;
//     for(uint8_t row = 0; row < 6; row++)
//     {
//         itoa(weekOfYear, buf, 10);
//         render->drawString(buf, startX + xMargin + colW/2+1, y + row * rowH , primaryColor, bgColor);
//         for(uint8_t col = 0; col < 7; col++)
//         {
//             if(month(currentTime) != month(time))
//             {
//                 render->setFontColor(TFT_DARKGREY);
//                 render->setBackgroundColor(bgColor);
//             }
//             else
//             {
//                 if(day(currentTime) == day(time))
//                 {
//                     render->setFontColor(TFT_BLACK);
//                     render->setBackgroundColor(TFT_DARKGREY);
//                     tft->fillSmoothRoundRect(startX + xMargin + col * colW + colW + 2, startY + row * rowH + rowH + 2, colW - 3, rowH - 3, 4, TFT_DARKGREY, bgColor);
//                     // tft->fillCircle(x + col * colW + 1, startY + rowH + row * rowH + rowH/2 + 1, rowH/2-3, TFT_SKYBLUE);
//                 }
//                 else
//                 {
//                     render->setFontColor(primaryColor);
//                     render->setBackgroundColor(bgColor);
//                 }
//             }
//             memset(buf, 0, sizeof(buf)/sizeof(buf[0]));
//             // dayNum = day(firstOfYear + weekOfYear * SECS_PER_WEEK);
//             itoa(day(currentTime), buf, 10);
//             render->drawString(buf, x + col * colW, y + row * rowH , primaryColor, bgColor);
//             currentTime += SECS_PER_DAY;
//         }
//         weekOfYear++;
//     }
// }
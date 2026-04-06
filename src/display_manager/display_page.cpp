#include "display_page.h"
#include "utils/utils.h"

// DisplayPage::DisplayPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor) :
// smallFont(smallFont), largeFont(largeFont), primaryColor(primaryColor), bgColor(bgColor) {}
DisplayPage::DisplayPage(uint16_t primaryColor, uint16_t bgColor) :
primaryColor(primaryColor),
bgColor(bgColor) {}

DisplayPage::DisplayPage(WidgetBase *(*widgets), uint8_t widgetCount, uint16_t primaryColor, uint16_t bgColor) :
primaryColor(primaryColor),
bgColor(bgColor)
{
    if(widgets != nullptr && widgetCount <= maxWidgets)
    {
        for(uint8_t num = 0; num < widgetCount; num++)
        {
            addWidget(widgets[num]);
        }
    }
}

void DisplayPage::display(bool fullUpdate)
{
    uint8_t startY = 0;
    uint8_t endY = startY;

    // if(fullUpdate)
    // {
    //     tft->fillScreen(bgColor);
    // }
    
    for(uint8_t num = 0; num < widgetCount; num++)
    {
        endY = startY + widgets[num]->getHeight(); // Check if fits
        
        if(endY < TFT_WIDTH && endY > startY)
        {
            widgets[num]->draw(fullUpdate);
            startY = endY;
        }
    }
}

bool DisplayPage::addWidget(WidgetBase *widget)
{
    if(widgetCount >= maxWidgets)
    {
        return false;
    }
    
    widgets[widgetCount] = widget;
    widgetCount++;

    return true;
}

void DisplayPage::init(TFT_eSPI *tft, OpenFontRender *render)
{
    for(uint8_t num = 0; num < widgetCount; num++)
    {
        widgets[num]->init(tft, render);
    }
}

// EventsPage::EventsPage(const CalManager *cal, uint16_t primaryColor, uint16_t bgColor) :
// DisplayPage(primaryColor, bgColor),
// calendar(cal) {}

// void EventsPage::display(TFT_eSPI *tft, OpenFontRender *render)
// {
//     uint8_t startY = 0;
//     uint8_t endY = startY;

//     tft->fillScreen(bgColor);

//     HeaderWidget header(TextWidget((datetimeToString(now(), true, false)).c_str(), 16, TFT_WHITE, TFT_BLACK), TFT_WHITE, TFT_BLACK);
//     // HeaderWidget header(TextWidget([](){return (datetimeToString(now(), true, false)).c_str();}, 16, TFT_WHITE, TFT_BLACK), TFT_WHITE, TFT_BLACK);
//     header.display(tft, render, 0, startY);
//     endY = startY + header.getHeight(tft, render);
//     startY = endY;
    
//     if(calendar != nullptr)
//     {
//         const std::vector<CalendarEvent> events = calendar->getEvents();

//         for(uint8_t num = 0; num < events.size(); num++)
//         {
//             // TextWidget(&events[num], &CalendarEvent::getDateStr, 16, TFT_WHITE, TFT_BLACK);
//             // TextWidget(nullptr, [](){return now()}, 16, TFT_WHITE, TFT_BLACK);
//             // TextWidget([events, num](){return events[num].getDateStr();}, 16, TFT_WHITE, TFT_BLACK);
//             // EventWidget widget(TextWidget(events[num].getDateStr(), 16, TFT_WHITE, TFT_BLACK),
//             //                    TextWidget(events[num].getSummary(), 24, TFT_WHITE, TFT_BLACK),
//             //                    TextWidget(events[num].getDurationStr(), 16, TFT_WHITE, TFT_BLACK, Align::Right),
//             //                    TFT_WHITE, TFT_BLACK);

//             EventWidget widget(&events[num], TFT_WHITE, TFT_BLACK);

//             endY = startY + widget.getHeight(tft, render); // Check if fits
            
//             if(endY < TFT_WIDTH && endY > startY)
//             {
//                 widget.display(tft, render, 0, startY);
//                 startY = endY;
//             }
//         }
        
//     }

// }
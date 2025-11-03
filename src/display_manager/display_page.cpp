#include "display_page.h"

DisplayPage::DisplayPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor) :
smallFont(smallFont), largeFont(largeFont), primaryColor(primaryColor), bgColor(bgColor) {}
DisplayPage::DisplayPage() {}
DisplayPage::~DisplayPage() {}

void DisplayPage::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startY)
{
    for(uint8_t num = 0; num < itemCount; num++)
    {
        items[num]->display(tft, render, startY);
        startY += items[num]->getHeight();
    }
}

EventsPage::EventsPage(uint8_t smallFont, uint8_t largeFont, uint16_t primaryColor, uint16_t bgColor) : 
DisplayPage(smallFont,  largeFont, primaryColor, bgColor)
{
    TextElement text = {
        .text = "Test",
        .fontSize = smallFont,
    };

    TextElement date{
        .text = "Tue, 11 Nov",
        .fontSize = smallFont,
    };

    TextElement summary = {
        .text = "Test long event name",
        .fontSize = largeFont,
    };

    TextElement duration = {
        .text = "Duration",
        .fontSize = smallFont,
    };

    static CalendarItem item1 = CalendarItem(date, summary, duration, primaryColor, bgColor);
    static CalendarItem item2 = CalendarItem(date, summary, duration, primaryColor, bgColor);
    static CalendarItem item3 = CalendarItem(date, summary, duration, primaryColor, bgColor);

    static DisplayItem *itemList[] = {
        &item1,
        &item2,
        &item3
    };

    items = itemList;
    itemCount = sizeof(itemList)/sizeof(itemList[0]);
}

EventsPage::EventsPage() : DisplayPage() {}
EventsPage::~EventsPage() {}
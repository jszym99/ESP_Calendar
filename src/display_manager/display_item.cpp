#include "display_item.h"

constexpr uint8_t xMargin = 4;
constexpr uint8_t xTextMargin = xMargin + 8;
constexpr uint8_t cellWidth = TFT_WIDTH-2*xMargin-1;
constexpr uint8_t tftHeight = 230; //TFT_HEIGHT;
constexpr uint8_t radius = 12;
constexpr uint8_t lineWidth = 1;

DisplayItem::DisplayItem(uint16_t fgColor, uint16_t  bgColor) : primaryColor(fgColor), bgColor(bgColor) {}
DisplayItem::~DisplayItem() {}

CalendarItem::CalendarItem(TextElement date, TextElement summary, TextElement duration, uint16_t fgColor, uint16_t  bgColor) :
date(date), summary(summary), duration(duration), DisplayItem(fgColor, bgColor)
{
    height =  date.fontSize + 1.5 * summary.fontSize + duration.fontSize;
}

CalendarItem::~CalendarItem() {}

void CalendarItem::display(TFT_eSPI *tft, OpenFontRender *render, uint8_t startY)
{
    uint8_t cellHeight = TFT_HEIGHT + radius - (startY + 0.75 * date.fontSize);

    // Draw frame
    tft->drawSmoothRoundRect(xMargin, startY + 0.75 * date.fontSize, radius, radius+lineWidth-1, cellWidth, cellHeight, primaryColor, bgColor);

tft->fillRect(xTextMargin + date.fontSize - xMargin, startY + 0.25 * date.fontSize + 1, render->getTextWidth(date.text.c_str()) + 2 * xMargin, render->getTextHeight(date.text.c_str()), bgColor);

    render->setFontSize(date.fontSize);
    render->drawString(date.text.c_str(), xTextMargin + date.fontSize, startY, primaryColor, bgColor);

    startY += date.fontSize;

    render->setFontSize(summary.fontSize);
    render->drawString(summary.text.c_str(), xTextMargin, startY, primaryColor, bgColor);

    startY += 1.25 * summary.fontSize;

    render->setFontSize(duration.fontSize);
    render->rdrawString(duration.text.c_str(), TFT_WIDTH - (xTextMargin+duration.fontSize), startY, primaryColor, bgColor);
}
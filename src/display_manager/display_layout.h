#ifndef DISPLAY_LAYOUT_H
#define DISPLAY_LAYOUT_H

#include <Arduino.h>
#include "display_page.h"

static char headerText[BUFSIZ];
TextWidget date(WidgetConfig{0,0, TFT_WIDTH, 24,TFT_WHITE,TFT_BLACK}, headerText, 16);
HeaderWidget header(WidgetConfig{0,0, TFT_WIDTH, 24,TFT_WHITE,TFT_BLACK}, date);



#endif /* DISPLAY_LAYOUT_H */
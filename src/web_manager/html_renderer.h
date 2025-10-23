#ifndef HTML_RENDERER_H
#define HTML_RENDERER_H

#include <string>
#include <vector>
#include "cal_manager/cal_manager.h"

class HtmlRenderer {
public:
    HtmlRenderer() = default;
    ~HtmlRenderer() = default;

    static std::string renderSettingsPage(const std::vector<Calendar> &calendars, const std::string &primaryHex, const std::string &bgHex);
};

#endif // HTML_RENDERER_H

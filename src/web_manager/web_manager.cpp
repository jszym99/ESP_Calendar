#include "web_manager.h"
#include "html_renderer.h"
#include <WiFi.h>
#include <utils/utils.h>

std::string rgb565ToHex(uint16_t c)
{
    uint8_t r5 = (c >> 11) & 0x1F;
    uint8_t g6 = (c >> 5) & 0x3F;
    uint8_t b5 = c & 0x1F;
    uint8_t r8 = (r5 << 3) | (r5 >> 2);
    uint8_t g8 = (g6 << 2) | (g6 >> 4);
    uint8_t b8 = (b5 << 3) | (b5 >> 2);
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", r8, g8, b8);
    return std::string(buf);
}

uint16_t hexTo565(std::string hex)
{
    long c = strtol(hex.c_str(), NULL, 16);
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8) & 0xFF;
    uint8_t b = c & 0xFF;
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

 // Helper: normalize and validate hex color string (allow leading '#')
std::string normalizeHex(String s)
{
    s.trim();
    if(s.startsWith("#")) s = s.substring(1);
    // convert to uppercase for consistency (not required)
    for(size_t i = 0; i < s.length(); ++i) s[i] = toupper(s[i]);
    return s.c_str();
};

bool isValidHex6(const std::string &s)
{
    if(s.length() != 6) return false;
    for(size_t i = 0; i < 6; ++i)
    {
        char c = s[i];
        bool ok = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
        if(!ok) return false;
    }
    return true;
};


WebManager::WebManager()
{
}

WebManager::~WebManager()
{
    server.stop();
}

void WebManager::init(CalManager *calManager)
{
    cal = calManager;
    registerRoutes();
}

void WebManager::registerRoutes()
{
    // Redirect root to settings page
    server.on("/", [this]() {
        server.sendHeader("Location", "/settings");
        server.send(303);
    });

    server.on("/health", [this](){
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/plain", "OK");
    });

    // Settings page - show form
    server.on("/settings", HTTP_GET, [this]() {
        if (!cal) {
            server.send(500, "text/plain", "CalManager not initialized");
            return;
        }

        std::string primaryHex = "#FFFFFF";
        std::string bgHex = "#000000";
        
        // DisplayManager stores colors as 16-bit RGB565; convert to #RRGGBB for the web UI
        primaryHex = rgb565ToHex(DisplayManager::getPrimaryColor());
        bgHex = rgb565ToHex(DisplayManager::getBackgroundColor());

        std::string html = HtmlRenderer::renderSettingsPage(cal->getCalendarList(), primaryHex, bgHex);
        server.send(200, "text/html; charset=utf-8", html.c_str());
    });

    // Settings handler - update calendar enabled/primary and colors
    server.on("/settings", HTTP_POST, [this]() {
        if(!cal) { server.send(500, "text/plain", "CalManager not initialized"); return; }

        const auto &cals = cal->getCalendarList();
        // Update enabled flags
        for(size_t i = 0; i < cals.size(); ++i)
        {
            String argName = "cal_enabled_" + String(i);
            if(server.hasArg(argName.c_str()))
            {
                cal->enableCalendar(i);
            }
            else
            {
                cal->disableCalendar(i);
            }
        }

        // Update colors
        if(server.hasArg("primaryColor") || server.hasArg("bgColor"))
        {
        bool changed = false;
            if(server.hasArg("primaryColor"))
            {
                std::string s = normalizeHex(server.arg("primaryColor")).c_str();
                if(isValidHex6(s))
                {
                    uint16_t col = hexTo565(s);
                    DisplayManager::setPrimaryColor(col);
                    changed = true;
                }
            }
            if(server.hasArg("bgColor"))
            {
                std::string s = normalizeHex(server.arg("bgColor")).c_str();
                if(isValidHex6(s))
                {
                    uint16_t col = hexTo565(s);
                    DisplayManager::setBackgroundColor(col);
                    changed = true;
                }
            }

            if(changed) {
                DisplayManager::display(); // Perform refresh if any color changed
            }
        }

        server.sendHeader("Location", "/settings");
        server.send(303);
    });

    // Refresh endpoint - set flag for main loop to call calendar.fetchCalendarList
    server.on("/refresh", HTTP_POST, [this]() {
        pendingRefresh = true;
        server.sendHeader("Location", "/settings");
        server.send(303);
    });
}

void WebManager::start()
{
    server.begin();
}

void WebManager::handle()
{
    server.handleClient();
}

bool WebManager::consumeRefreshRequest()
{
    if(pendingRefresh)
    {
        pendingRefresh = false;
        return true;
    }
    return false;
}

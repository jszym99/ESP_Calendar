#include "web_manager.h"
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

WebManager::WebManager() {}

WebManager::~WebManager()
{
    server.stop();
}

void WebManager::init(CalManager *calManager)
{
    cal = calManager;

    if (!LittleFS.begin(true)) {
        Serial.println("WebManager: LittleFS mount failed");
    } else {
        Serial.println("WebManager: LittleFS mounted");
    }

    registerRoutes();
}

void WebManager::registerRoutes()
{
    // Redirect root to settings page
    server.on("/", HTTP_GET, [this]() {
        server.sendHeader("Location", "/settings");
        server.send(303);
    });

    server.on("/health", HTTP_GET, [this]() {
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/plain", "OK");
    });

    // Settings page — serve static HTML from LittleFS
    server.on("/settings", HTTP_GET, [this]() {
        File f = LittleFS.open("/index.html", "r");
        if (!f) {
            server.send(500, "text/plain", "index.html not found on filesystem");
            return;
        }
        server.streamFile(f, "text/html");
        f.close();
    });

    // API: return calendar state as JSON
    server.on("/api/state", HTTP_GET, [this]() {
        if (!cal) {
            server.send(500, "application/json", "{\"error\":\"not ready\"}");
            return;
        }

        JsonDocument doc;
        Calendar calendars[maxCalendarCount];
        uint8_t count = cal->getCalendarList(calendars, maxCalendarCount);

        JsonArray arr = doc["calendars"].to<JsonArray>();
        for (uint8_t i = 0; i < count; i++) {
            JsonObject obj = arr.add<JsonObject>();
            obj["id"]          = calendars[i].getId();
            obj["summary"]     = calendars[i].getSummary();
            obj["description"] = calendars[i].getDescription();
            obj["primary"]     = calendars[i].isPrimary();
            obj["enabled"]     = calendars[i].isEnabled();
        }

        String out;
        serializeJson(doc, out);
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "application/json", out);
    });

    // Settings POST — update calendar toggles
    server.on("/settings", HTTP_POST, [this]() {
        if (!cal) {
            server.send(500, "text/plain", "CalManager not initialized");
            return;
        }

        Calendar *calPtrs[maxCalendarCount];
        uint8_t count = cal->getCalendarPtrList(calPtrs, maxCalendarCount);

        bool toggled = false;
        for (uint8_t i = 0; i < count; i++) {
            std::string argName = std::string("cal_enabled_") + calPtrs[i]->getId();
            bool nowEnabled = server.hasArg(argName.c_str());
            if (nowEnabled != calPtrs[i]->isEnabled()) toggled = true;
            nowEnabled ? calPtrs[i]->enable() : calPtrs[i]->disable();
        }
        if (toggled) cal->fetchEvents();

        server.sendHeader("Location", "/settings");
        server.send(303);
    });

    // Refresh endpoint
    server.on("/refresh", HTTP_POST, [this]() {
        pendingRefresh = true;
        server.sendHeader("Cache-Control", "no-store");
        server.send(200, "text/plain", "OK");
    });
}

void WebManager::start()
{
    ElegantOTA.begin(&server);
    server.begin();
    Serial.println("WebManager: HTTP server started (ElegantOTA at /update)");
}

void WebManager::handle()
{
    server.handleClient();
    ElegantOTA.loop();
}

bool WebManager::consumeRefreshRequest()
{
    if (pendingRefresh) {
        pendingRefresh = false;
        return true;
    }
    return false;
}

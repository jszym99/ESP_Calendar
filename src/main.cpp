#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "config.h"
#include "auth_manager/auth_manager.h"
#include "cal_manager/cal_manager.h"
#include "display_manager/display_manager.h"
#include "touch_button/touch_button.h"
#include "utils/utils.h"
// #include "web_manager/web_manager.h"

#define TOUCH_PIN T9

constexpr uint64_t updateTime = 60 * 1000; // 1 min

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET); // TODO: Read cliend ID and Secret from EEPROM
CalManager calendar = CalManager(&oauth);
TouchButton touchBtn = TouchButton(TOUCH_PIN);
// WebManager web;

HeaderWidget header(WidgetConfig{0,0, TFT_WIDTH, 24,TFT_WHITE,TFT_BLACK}, &WiFi);
DynamicEventListWidget eventList(WidgetConfig{0,header.getHeight(), TFT_WIDTH, TFT_HEIGHT,TFT_WHITE,TFT_BLACK}, &calendar);
WidgetBase *dynWidgets[] =
{
    &header,
    &eventList,
};
DisplayPage dynPage(dynWidgets, sizeof(dynWidgets)/sizeof(dynWidgets[0]), TFT_WHITE, TFT_BLACK);
// MonthWidget monthWidget(TFT_WHITE, TFT_BLACK);
// WidgetBase *monthWidgets[] =
// {
//     &header,
//     &monthWidget,
// };
// DisplayPage monthPage{monthWidgets, sizeof(monthWidgets)/sizeof(monthWidgets[0]), TFT_WHITE, TFT_BLACK};

void setup()
{
	Serial.begin(115200);

    // Initialize hardware
    DisplayManager::addPage(&dynPage);
    // DisplayManager::addPage(&monthPage);
    DisplayManager::init();

    touchBtn.init();
    
    // Start WiFi connection
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Initialize web manager (routes can be registered now)
    // web.init(&calendar);
}

void loop()
{
    static bool isConnected = false;
    static uint64_t lastUpdate = 0;
    static uint64_t lastToken = 0;

    if (touchBtn.isClick())
    {
        DisplayManager::nextPage();
    }
        else if(touchBtn.isClick(2)) // Cycle brightness on touch
    {
        uint8_t displayBrightness = DisplayManager::getBrightness();
        if(displayBrightness == maxDisplayBrightness)
        {
            DisplayManager::setBrightness(0);
        }
        else
        {
            uint8_t newBrightness = displayBrightness + (maxDisplayBrightness+1)/4;
            if(newBrightness > displayBrightness)
            {
                DisplayManager::setBrightness(newBrightness);
            }
            else
            {
                DisplayManager::setBrightness(maxDisplayBrightness);
            }
        }
        Serial.println("Touch button clicked.");
        Serial.print("Display brightness:");
        Serial.print(((DisplayManager::getBrightness()*100)/maxDisplayBrightness));
        Serial.println("%");
    }

    // Connected to WiFi
	if(WiFi.status() == WL_CONNECTED && isConnected == false)
	{
        // Print Network status
        Serial.println("\033[0;32mWiFi connected!\033[0m");
        Serial.print("ESP32 IP Address: ");
        Serial.println(WiFi.localIP());

        // Start NTP
        timeClient.begin();
        timeClient.setTimeOffset(3600);

        if(oauth.authenticate())
        {
            lastToken = millis();
        }
        
        calendar.fetchCalendarList();

        // Start HTTP server once we have WiFi
        // web.start();

		isConnected = true;
	}
	
    // Try to connect repeatedly
	if(WiFi.status() != WL_CONNECTED)
	{
        Serial.print("Connecting to: ");
        Serial.println(WIFI_SSID);

		delay(500);
		isConnected = false;
	}
    
    // WiFi connected - normal mode
    if(isConnected)
    {
        // Authenticate
        if(lastToken + oauth.getTokenLifetime() * 1000 < millis() || lastToken == 0)
        {
            if(oauth.authenticate())
            {
                lastToken = millis();
            }
        }

        // Perform update
        if(lastUpdate + updateTime < millis() || lastUpdate == 0)
        {
                // Fetch time from NTP
            while(!timeClient.update()) {
                timeClient.forceUpdate();
            }

            // Update system time
            setTime(timeClient.getEpochTime());
            // Serial.println(datetimeToString(now(), true, false).c_str());
            Serial.println(datetimeStr(now()));
        
            // Get events
            calendar.fetchEvents();
            
            // Update events
            eventList.update();

            // Update current display page
            DisplayManager::display();
            
            lastUpdate = millis();
        }
        // Process web server client connections
        // web.handle();

        // If web UI requested calendar refresh, perform it here with OAuth token
        // if(web.consumeRefreshRequest())
        // {
        //     calendar.fetchCalendarList();
        // }
    }
    else // WiFi not connected - AP mode
    {
        // TODO: AP mode
    }

}
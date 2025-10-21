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

#define TOUCH_PIN T9

constexpr uint64_t updateTime = 60 * 1000; // 1 min

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET); // TODO: Read cliend ID and Secret from EEPROM
DisplayManager display = DisplayManager();
CalManager calendar = CalManager(&display);
TouchButton touchBtn = TouchButton(TOUCH_PIN);

void update()
{
    // Fetch time from NTP
    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }

    // Update system time
    setTime(timeClient.getEpochTime());
    Serial.println(datetimeToString(now(), true, false).c_str());

    // Get events
    calendar.fetchEvents(oauth.getAccessToken());
    calendar.showEvents();
    display.displayHeader();
}

void setup()
{
    // Initialize hardware
    display.init();
    touchBtn.init();
    
    // Start WiFi connection
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.begin(115200);
}

void loop()
{
    static bool isConnected = false;
    static uint64_t lastUpdate = 0;
    static uint64_t lastToken = 0;

    // Cycle brightness on touch
    if(touchBtn.isClick())
    {
        uint8_t displayBrightness = display.getBrightness();
        if(displayBrightness == maxDisplayBrightness)
        {
            display.setBrightness(0);
        }
        else
        {
            uint8_t newBrightness = displayBrightness + (maxDisplayBrightness+1)/4;
            if(newBrightness > displayBrightness)
            {
                display.setBrightness(newBrightness);
            }
            else
            {
                display.setBrightness(maxDisplayBrightness);
            }
        }
        Serial.println("Touch button clicked.");
        Serial.print("Display brightness:");
        Serial.print(((display.getBrightness()*100)/maxDisplayBrightness));
        Serial.println("%");

    }

    // Connected to WiFi
	if(WiFi.status() == WL_CONNECTED && isConnected == false)
	{
        // Print Network status
        Serial.println("WiFi connected!");
        Serial.print("ESP32 IP Address: ");
        Serial.println(WiFi.localIP());

        // Start NTP
        timeClient.begin();
        timeClient.setTimeOffset(7200);

        if(oauth.authenticate())
        {
            lastToken = millis();
        }
        
        calendar.fetchCalendarList(oauth.getAccessToken());

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
            update();
            lastUpdate = millis();
        }
    }
    else // WiFi not connected - AP mode
    {
        // TODO: AP mode
    }

}
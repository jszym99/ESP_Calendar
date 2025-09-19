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
#define LED 2

#define TOUCH_PIN T9

constexpr uint64_t updateTime = 60 * 1000; // 1 min

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET);
CalManager calendar = CalManager([](){ return oauth.getAccessToken();});
DisplayManager display;
TouchButton touchBtn = TouchButton(TOUCH_PIN);

void update()
{
    static std::vector<eventData> previousEvents = {};
    static std::vector<eventData> events = {};

    // Fetch time from NTP
    while(!timeClient.update()) {
        timeClient.forceUpdate();
    }

    // Update system time
    time_t systemTime = timeClient.getEpochTime();
    setTime(systemTime);
    Serial.println(DATETIME_STR(now(), true, false));

    // Get events
    calendar.fetchEvents();
    events = calendar.getEvents();
    
    // Update displayed events if any changed
    if(previousEvents != events)
    {
        display.displayEvents(events);
        previousEvents = events;
    }
}

void setup()
{
	// pinMode(LED, OUTPUT);
    display.setBrightness(maxDisplayBrightness/2);
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

        oauth.init(); // Init oauth
        update(); // Update display

		// digitalWrite(LED, HIGH);
		isConnected = true;
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
        Serial.print("Connecting to: ");
        Serial.println(WIFI_SSID);

		// digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
    
    // WiFi connected - normal mode
    if(isConnected)
    {
        // Authenticate
        if(lastToken + oauth.getTokenLifetime() * 1000 < millis())
        {
            oauth.init();
            lastToken = millis();
        }

        // Perform update
        if(lastUpdate + updateTime < millis())
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
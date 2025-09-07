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
#include "utils/utils.h"
#define LED 2

bool isConnected = false;
uint64_t lastUpdate = 0, lastToken = 0;
constexpr uint64_t updateTime = 5 * 1000; // 5 sec

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET);
CalManager calendar = CalManager([](){ return oauth.getAccessToken();});
DisplayManager display;

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
    
    if(previousEvents != events)
    {
        display.displayEvents(events);
        previousEvents = events;
    }
}

void setup()
{
	pinMode(OUTPUT, LED);
    
    // Start WiFi connection
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.begin(115200);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

    // Print Network status
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Start NTP
    timeClient.begin();
    timeClient.setTimeOffset(7200);
}

void loop()
{
    // Connected to WiFi
	if(WiFi.status() == WL_CONNECTED && isConnected == false)
	{
		digitalWrite(LED, HIGH);
		isConnected = true;
        oauth.init();
        update();
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
    
    // Authenticate
    if(isConnected && lastToken + oauth.getTokenLifetime() * 1000 < millis())
    {
        oauth.init();
        lastToken = millis();
    }

    // Perform update
    if(isConnected && lastUpdate + updateTime < millis())
    {
        update();
        lastUpdate = millis();
    }
}
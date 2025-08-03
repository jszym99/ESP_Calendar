#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "SPI.h"
#include "TFT_eSPI.h"

#include "config.h"
#include "auth_manager/auth_manager.h"
#include "cal_manager/cal_manager.h"
#include "utils/utils.h"

#define LED 2

bool isConnected = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft = TFT_eSPI();  // Invoke library
AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET);
CalManager calendar = CalManager([](){ return oauth.getAccessToken();});


String formatDatetime(String stringDate) {
    int tIndex = stringDate.indexOf('T');
    if (tIndex != -1) {
        String datePart = stringDate.substring(0, tIndex);
        String timePart = stringDate.substring(tIndex + 1);

        // Remove 'Z' if present at the end
        if (timePart.endsWith("Z")) {
            timePart = timePart.substring(0, timePart.length() - 1);
        }
        // Remove timezone offset if present (e.g., +02:00 or -05:00)
        int plusIndex = timePart.indexOf('+');
        int minusIndex = timePart.indexOf('-');
        int tzIndex = (plusIndex != -1) ? plusIndex : ((minusIndex > 0) ? minusIndex : -1);
        if (tzIndex != -1) {
            timePart = timePart.substring(0, tzIndex);
        }
        // Remove milliseconds if present (e.g., 15:30:00.000)
        int dotIndex = timePart.indexOf('.');
        if (dotIndex != -1) {
            timePart = timePart.substring(0, dotIndex);
        }

        // Remove seconds if present (e.g., 15:30:00 -> 15:30)
        int colonIndex = timePart.indexOf(':');
        if (colonIndex != -1) {
            int secondColon = timePart.indexOf(':', colonIndex + 1);
            if (secondColon != -1) {
                timePart = timePart.substring(0, secondColon);
            }
        }

        // Convert datePart from yyyy-mm-dd to dd-mm-yyyy
        int firstDash = datePart.indexOf('-');
        int secondDash = datePart.indexOf('-', firstDash + 1);
        String year = datePart.substring(0, firstDash);
        String month = datePart.substring(firstDash + 1, secondDash);
        String day = datePart.substring(secondDash + 1);

        String formattedDate = day + "-" + month + "-" + year;
        return formattedDate + " " + timePart;
    } else {
        // Only date is present, convert yyyy-mm-dd to dd-mm-yyyy
        int firstDash = stringDate.indexOf('-');
        int secondDash = stringDate.indexOf('-', firstDash + 1);
        if (firstDash != -1 && secondDash != -1) {
            String year = stringDate.substring(0, firstDash);
            String month = stringDate.substring(firstDash + 1, secondDash);
            String day = stringDate.substring(secondDash + 1);
            return day + "-" + month + "-" + year;
        } else {
            return stringDate;
        }
    }
}

void setup()
{
	pinMode(OUTPUT, LED);
    
    // Start WiFi connection
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.begin(115200);

    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    tft.setTextFont(2);
    // tft.setTextSize(2);
    tft.println("Connecting to:");
    tft.println(WIFI_SSID);
    tft.setTextColor(TFT_WHITE);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

    // Print Network status
    tft.setTextColor(TFT_GREEN);
    tft.println("WiFi connected!");
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

        // Fetch time from NTP
        while(!timeClient.update()) {
            timeClient.forceUpdate();
        }

        // Update system time
        time_t systemTime = timeClient.getEpochTime();
        setTime(systemTime);
        tft.setTextColor(TFT_WHITE);
        Serial.println(DATETIME_STR(now(), false));
        tft.println(DATETIME_STR(now(), false));

        // Authenticate
        oauth.init();
        tft.setTextColor(TFT_GREEN);
        tft.println("Authenticated!");

        // Get events
        calendar.fetchEvents();
        std::vector<eventData> events = calendar.getEvents();

        // Display event error
        if(events.empty())
        {
            tft.setTextColor(TFT_RED);
            tft.println("No events found!");
        }

        // Display events
        tft.setTextColor(TFT_WHITE);
        for(int i = 0; i < events.size(); i++)
        {
            tft.println(DATETIME_STR(events[i].startDate, events[i].isAllDay));
            tft.println(events[i].summary);
        }
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
}
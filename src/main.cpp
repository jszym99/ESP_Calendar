#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include "auth_manager/auth_manager.h"
#include "time.h"

#define LED 2


bool isConnected = false;
String accessToken;
String formattedDate;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft = TFT_eSPI();  // Invoke library
AuthManager oauth = AuthManager(CLIENT_ID, CLIENT_SECRET);


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

void calendarRequest() {
    HTTPClient http;
    JsonDocument response;
	http.begin("https://www.googleapis.com/calendar/v3/calendars/primary/events?maxResults=5&orderBy=startTime&singleEvents=true&timeMin=" + formattedDate);
    http.addHeader("Authorization",  "Bearer " + oauth.getAccessToken());

    int httpResponseCode = http.GET();
    String response_string = http.getString();
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Response: " + response_string);

    deserializeJson(response, response_string);

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    for(int i = 0; i < response["items"].size(); i++)
    {
        if(response["items"][i]["start"]["dateTime"].is<String>())
            tft.println(formatDatetime(response["items"][i]["start"]["dateTime"].as<String>()));
        else
            tft.println(formatDatetime(response["items"][i]["start"]["date"].as<String>()));
        tft.println(response["items"][i]["summary"].as<String>());
    }
}


void setup()
{
	pinMode(OUTPUT, LED);
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.begin(115200);

    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    tft.setTextFont(2);
    // tft.setTextSize(2);
    tft.println("Connecting to:");
    tft.setTextColor(TFT_BLUE);
    tft.println(WIFI_SSID);
    tft.setTextColor(TFT_WHITE);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

    tft.println("WiFi connected!");
    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

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

        while(!timeClient.update()) {
            timeClient.forceUpdate();
        }
        formattedDate = timeClient.getFormattedDate();
        Serial.println(formatDatetime(formattedDate));
        tft.println(formatDatetime(formattedDate));

        // Send POST request
        oauth.init();
        tft.println("Authenticated!");
        calendarRequest();
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
}
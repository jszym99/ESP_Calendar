#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "config.h"
#include "SPI.h"
#include "TFT_eSPI.h"
#include "oauth/oauth.h"


#define LED 2


bool isConnected = false;
String accessToken;
String formattedDate;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft = TFT_eSPI();  // Invoke library
OAuth oauth = OAuth(CLIENT_ID, CLIENT_SECRET);

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

    for(int i = 0; i < response["items"].size(); i++)
    {
        tft.println(response["items"][i]["summary"].as<String>());
        if(response["items"][i]["start"].containsKey("dateTime"))
            tft.println(response["items"][i]["start"]["dateTime"].as<String>());
        else
            tft.println(response["items"][i]["start"]["date"].as<String>());
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

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

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
        Serial.println(formattedDate);

        // Send POST request
        oauth.init();
        calendarRequest();
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
}
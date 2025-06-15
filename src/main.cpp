#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// #define WIFI_SSID WIFI_SSID
// #define WIFI_PASSWORD WIFI_PASSWORD

// #define CLIENT_ID CLIENT_ID
// #define CLIENT_SECRET CLIENT_SECRET

#define LED 2

bool isConnected = false;

void sendPostRequest(String client_id)
{
    HTTPClient http;
    String postData = "client_id=" + client_id + "&scope=https://www.googleapis.com/auth/calendar.readonly";
	JsonDocument response;

    http.begin("https://oauth2.googleapis.com/device/code");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode > 0)
    {
        String response_string = http.getString();
		deserializeJson(response, response_string);
        Serial.println("HTTP Response Code: " + String(httpResponseCode));
        Serial.println("Response: " + response_string);
    }
    else
    {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
    }
	
	http.end();
	
    String response_string;

	do
	{
		delay(10000);
		http.begin("https://oauth2.googleapis.com/token");
		http.addHeader("Content-Type", "application/x-www-form-urlencoded");

		String code = response["device_code"];

		postData = "client_id=" + client_id + "&client_secret=" CLIENT_SECRET + "&device_code=" + code + "&grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code";

		httpResponseCode = http.POST(postData);
        response_string = http.getString();
		Serial.println("Waiting for authorization.");
		Serial.println("HTTP Response Code: " + String(httpResponseCode));
		Serial.println("Response: " + response_string);

        
		
	} while (httpResponseCode != 200);

    http.end();

    deserializeJson(response, response_string);
	
	http.begin("https://www.googleapis.com/calendar/v3/calendars/primary");
    String auth = response["access_token"];
    Serial.println("Access token: " + auth);
    http.addHeader("Authorization",  "Bearer " + auth);

    httpResponseCode = http.GET();
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Response: " + http.getString());

}

void setup()
{
	pinMode(OUTPUT, LED);
	WiFi.setHostname("ESP32");
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

	Serial.begin(115200);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("WiFi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

	server.begin(); // Start the server

	// Send POST request
    sendPostRequest(CLIENT_ID);
}

void loop()
{
	if(WiFi.status() == WL_CONNECTED)
	{
		digitalWrite(LED, HIGH);
		isConnected = true;
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
}
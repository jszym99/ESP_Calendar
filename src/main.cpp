#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "config.h"
#include "SPI.h"
#include "TFT_eSPI.h"

// #define WIFI_SSID WIFI_SSID
// #define WIFI_PASSWORD WIFI_PASSWORD

// #define CLIENT_ID CLIENT_ID
// #define CLIENT_SECRET CLIENT_SECRET

#define LED 2
#define EEPROM_SIZE 512
#define REFRESH_TOKEN_ADDRESS 0


bool isConnected = false;
String accessToken;
String formattedDate;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

TFT_eSPI tft = TFT_eSPI();  // Invoke library

void calendarRequest() {
    HTTPClient http;
    JsonDocument response;
	http.begin("https://www.googleapis.com/calendar/v3/calendars/primary/events?maxResults=5&orderBy=startTime&singleEvents=true&timeMin=" + formattedDate);
    http.addHeader("Authorization",  "Bearer " + accessToken);

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

void saveRefreshToken(String refreshToken) {
    Serial.println("Refresh token: " + refreshToken);
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < refreshToken.length(); i++) {
        EEPROM.write(REFRESH_TOKEN_ADDRESS + i, refreshToken[i]);
    }
    EEPROM.write(REFRESH_TOKEN_ADDRESS + refreshToken.length(), '\0'); // Null-terminate the string
    EEPROM.commit();
}

String readRefreshToken() {
    EEPROM.begin(EEPROM_SIZE);
    String refreshToken = "";
    char ch;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        ch = EEPROM.read(REFRESH_TOKEN_ADDRESS + i);
        if (ch == '\0') break; // Stop reading at null terminator
        refreshToken += ch;
    }
    return refreshToken;
}

bool authenticateWithRefreshToken(String refreshToken, String client_id) {
    HTTPClient http;
    http.begin("https://oauth2.googleapis.com/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "client_id=" + client_id + "&client_secret=" CLIENT_SECRET + "&refresh_token=" + refreshToken + "&grant_type=refresh_token";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode == 200) {
        String response_string = http.getString();
        JsonDocument response;
        deserializeJson(response, response_string);
        Serial.println("Authenticated using refresh token.");
        Serial.println("Response: " + response_string);

        // Save new refresh token if provided
        if (response.containsKey("refresh_token")) {
            saveRefreshToken(response["refresh_token"].as<String>());
        }

        accessToken = response["access_token"].as<String>();

        http.end();
        return true;
    } else {
        Serial.println("Failed to authenticate using refresh token. HTTP Response Code: " + String(httpResponseCode));
        http.end();
        return false;
    }
}

void authenticateWithAccessToken(String client_id)
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

    // Save refresh token if provided
    if (response.containsKey("refresh_token")) {
        saveRefreshToken(response["refresh_token"].as<String>());
    }

    accessToken = response["access_token"].as<String>();
}

void authenticate()
{
    String refreshToken = readRefreshToken();
    if (!refreshToken.isEmpty()) {
        Serial.println("Refresh token: " + refreshToken);
        if (authenticateWithRefreshToken(refreshToken, CLIENT_ID)) {
            return;
        }
    }

    Serial.println("No valid refresh token found.");
    authenticateWithAccessToken(CLIENT_ID);
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
        authenticate();
        calendarRequest();
	}
	
	if(WiFi.status() != WL_CONNECTED)
	{
		digitalWrite(LED, !digitalRead(LED));
		delay(500);
		isConnected = false;
	}
}
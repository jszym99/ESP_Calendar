#include "oauth.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define REFRESH_TOKEN_ADDRESS 0

const String codeURL = "https://oauth2.googleapis.com/device/code";
const String tokenURL = "https://oauth2.googleapis.com/token";
const String scope = "https://www.googleapis.com/auth/calendar.readonly";

const String headerName = "Content-Type";
const String headerValue = "application/x-www-form-urlencoded";


OAuth::OAuth(String _clientId, String _clientSecret)
{
    clientId = _clientId;
    clientSecret = _clientSecret;
    accessToken = "";
    refreshToken = "";
}

OAuth::~OAuth()
{
}

void OAuth::init()
{
    readRefreshToken();
    if (!refreshToken.isEmpty()) {
        Serial.println("Refresh token: " + refreshToken);
        if (authWithRefreshToken()) {
            return;
        }
    }

    authWithAccessToken();
}

bool OAuth::authWithAccessToken()
{
    HTTPClient http;
    JsonDocument response;
    String postData = "client_id=" + clientId + "&scope=" + scope;

    /* Request device and user codes */
    
    http.begin(codeURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode == 200)
    {
        String response_string = http.getString();
		deserializeJson(response, response_string);
        Serial.println("HTTP Response Code: " + String(httpResponseCode));
        Serial.println("Response: " + response_string);
        http.end();
    }
    else
    {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
        http.end();
        return false;
    }

    /* Poll authorization server */
    String response_string;

    do
    {
        delay(5000);

        http.begin(tokenURL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String deviceCode = response["device_code"];

		postData = "client_id=" + clientId + "&client_secret=" + clientSecret + "&device_code=" + deviceCode + "&grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code";

		httpResponseCode = http.POST(postData);
        response_string = http.getString();
		Serial.println("Waiting for authorization.");
		Serial.println("HTTP Response Code: " + String(httpResponseCode));
		Serial.println("Response: " + response_string);
    }
    while (httpResponseCode != 200);

    http.end();

    deserializeJson(response, response_string);

    // Save access and refresh tokens
    accessToken = response["access_token"].as<String>();
    refreshToken = response["refresh_token"].as<String>();
    saveRefreshToken(); // Save to EEPROM

    return true;
    
}

bool OAuth::authWithRefreshToken()
{
     HTTPClient http;
    http.begin("https://oauth2.googleapis.com/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String postData = "client_id=" + clientId + "&client_secret=" + clientSecret + "&refresh_token=" + refreshToken + "&grant_type=refresh_token";
    int httpResponseCode = http.POST(postData);

    if (httpResponseCode == 200) {
        String response_string = http.getString();
        JsonDocument response;
        deserializeJson(response, response_string);
        Serial.println("Authenticated using refresh token.");
        Serial.println("Response: " + response_string);

        // Save access and refresh tokens
        accessToken = response["access_token"].as<String>();
        refreshToken = response["refresh_token"].as<String>();
        saveRefreshToken(); // Save to EEPROM

        http.end();
        return true;
    } else {
        Serial.println("Failed to authenticate using refresh token. HTTP Response Code: " + String(httpResponseCode));
        http.end();
        return false;
    }
}


/* TODO - better memory management */
void OAuth::saveRefreshToken()
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < refreshToken.length(); i++) {
        EEPROM.write(REFRESH_TOKEN_ADDRESS + i, refreshToken[i]);
    }
    EEPROM.write(REFRESH_TOKEN_ADDRESS + refreshToken.length(), '\0'); // Null-terminate the string
    EEPROM.commit();
}

void OAuth::readRefreshToken()
{
    EEPROM.begin(EEPROM_SIZE);
    String token = "";
    char ch;
    for (int i = 0; i < EEPROM_SIZE; i++) {
        ch = EEPROM.read(REFRESH_TOKEN_ADDRESS + i);
        if (ch == '\0') break; // Stop reading at null terminator
        token += ch;
    }
    refreshToken = token;
}
#include "auth_manager.h"
#include <HTTPClient.h>
#include <EEPROM.h>

#define EEPROM_SIZE 512
#define REFRESH_TOKEN_ADDRESS 0

const String codeURL = "https://oauth2.googleapis.com/device/code";
const String tokenURL = "https://oauth2.googleapis.com/token";
const String scope = "https://www.googleapis.com/auth/calendar.readonly";

const String headerName = "Content-Type";
const String headerValue = "application/x-www-form-urlencoded";


AuthManager::AuthManager(String _clientId, String _clientSecret)
{
    clientId = _clientId;
    clientSecret = _clientSecret;
    accessToken = "";
    refreshToken = "";
    tokenLifetime = 0u;
}

AuthManager::~AuthManager()
{
}

void AuthManager::init()
{
    // Read refresh token from EEPROM if necessary
    if(refreshToken.isEmpty())
    {
        readRefreshToken();
    }

    // Authenticate with refresh token
    if (!refreshToken.isEmpty()) {
        Serial.println("Refresh token: " + refreshToken);
        if (authWithRefreshToken()) {
            return;
        }
    }

    authWithAccessToken();
}

bool AuthManager::authWithAccessToken()
{
    HTTPClient http;
    JsonDocument response;
    String postData = "client_id=" + clientId + "&scope=" + scope;
    String response_string;
    String deviceCode;
    uint16_t interval = 5000;

    /* Request device and user codes */
    http.begin(codeURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode == 200)
    {
        response_string = http.getString();
		deserializeJson(response, response_string);
        Serial.println("HTTP Response Code: " + String(httpResponseCode));
        Serial.println("Response: " + response_string);
        http.end();

        if(response["interval"].is<uint16_t>())
        {
            interval = response["interval"].as<uint16_t>() * 1000;
        }
    }
    else
    {
        Serial.println("Error on sending POST: " + String(httpResponseCode));
        http.end();
        return false;
    }

    /* Poll authorization server */
    do
    {
        delay(interval);

        http.begin(tokenURL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        deviceCode = response["device_code"].as<String>();

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

    if(response["access_token"].is<String>() && 
       response["refresh_token"].is<String>() &&
       response["expires_in"].is<uint16_t>())
    {
        accessToken = response["access_token"].as<String>();
        refreshToken = response["refresh_token"].as<String>();
        tokenLifetime = response["expires_in"];
        saveRefreshToken(); // Save to EEPROM
    }
    else
    {
        return false;
    }

    return true;
    
}

bool AuthManager::authWithRefreshToken()
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

        // Save access tokens
        if(response["access_token"].is<String>() &&
           response["expires_in"].is<uint16_t>())
        {
            accessToken = response["access_token"].as<String>();
            tokenLifetime = response["expires_in"];
        }
        else
        {
            return false;
        }

        http.end();
        return true;
    } else {
        Serial.println("Failed to authenticate using refresh token. HTTP Response Code: " + String(httpResponseCode));
        http.end();
        return false;
    }
}


/* TODO - better memory management */
void AuthManager::saveRefreshToken()
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < refreshToken.length(); i++) {
        EEPROM.write(REFRESH_TOKEN_ADDRESS + i, refreshToken[i]);
    }
    EEPROM.write(REFRESH_TOKEN_ADDRESS + refreshToken.length(), '\0'); // Null-terminate the string
    EEPROM.commit();
}

void AuthManager::readRefreshToken()
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
#include "auth_manager.h"
#include <HTTPClient.h>
#include <EEPROM.h>
#include "utils/utils.h"

#define EEPROM_SIZE 512
#define REFRESH_TOKEN_ADDRESS 0

const char codeURL[] = "https://oauth2.googleapis.com/device/code";
const char tokenURL[] = "https://oauth2.googleapis.com/token";
const char scope[] = "https://www.googleapis.com/auth/calendar.readonly";

const char headerName[] = "Content-Type";
const char headerValue[] = "application/x-www-form-urlencoded";


AuthManager::AuthManager(std::string _clientId, std::string _clientSecret)
{
    clientId = _clientId;
    clientSecret = _clientSecret;
    // accessToken = "";
    memset(accessToken, 0, sizeof(accessToken)/sizeof(accessToken[0]));
    // refreshToken = "";
    memset(refreshToken, 0, sizeof(refreshToken)/sizeof(refreshToken[0]));
    tokenLifetime = 0u;
}

AuthManager::~AuthManager()
{
}

/**
 * @brief Generate new auth token.
 * 
 * @return true Generated new token successfully.
 * @return false Failed to generate new token.
 */
bool AuthManager::authenticate()
{
    // Read refresh token from EEPROM if necessary
    if(strlen(refreshToken) == 0)
    {
        readRefreshToken();
    }

    // Authenticate with refresh token
    if (strlen(refreshToken) != 0) {
        Serial.print("Refresh token: ");
        Serial.println(refreshToken);
        if (authWithRefreshToken()) {
            return true;
        }
    }

    return authWithDeviceCode();
}

/**
 * @brief Generate new auth token using device code.
 * 
 * @return true Generated new token successfully.
 * @return false Failed to generate new token.
 */
bool AuthManager::authWithDeviceCode()
{
    HTTPClient http;
    JsonDocument response;
    std::string postData = "client_id=" + clientId + "&scope=" + scope;
    String responseString;
    std::string deviceCode;
    uint16_t interval = 5000;

    /* Request device and user codes */
    http.begin(codeURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Get response and code
    int httpResponseCode = http.POST(postData.c_str());
    responseString = http.getString();
    
    printHttpResp(httpResponseCode, responseString);

    if (httpResponseCode == HTTP_CODE_OK)
    {

		deserializeJson(response, responseString);

        http.end();

        if(response["interval"].is<uint16_t>())
        {
            interval = response["interval"].as<uint16_t>() * 1000;
        }
    }
    else
    {
        Serial.println("\33[0;31mError on sending POST: " + String(httpResponseCode) + "\33[0m");
        http.end();
        return false;
    }

    /* Poll authorization server */
    do
    {
        delay(interval);

        http.begin(tokenURL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        deviceCode = response["device_code"].as<std::string>();

		postData = "client_id=" + clientId + "&client_secret=" + clientSecret + "&device_code=" + deviceCode + "&grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Adevice_code";

        // Get response and code
		httpResponseCode = http.POST(postData.c_str());
        responseString = http.getString();

        printHttpResp(httpResponseCode, responseString);

		Serial.println("Waiting for authorization.");
    }
    while (httpResponseCode != HTTP_CODE_OK); //TODO: add timeout

    http.end();

    deserializeJson(response, responseString);

    if(response["access_token"].is<const char *>() && 
       response["refresh_token"].is<const char *>() &&
       response["expires_in"].is<uint16_t>())
    {
        strcpy(accessToken, response["access_token"].as<const char *>());
        // refreshToken = response["refresh_token"].as<std::string>();
        strcpy(refreshToken, response["refresh_token"].as<const char *>());
        tokenLifetime = response["expires_in"];
        saveRefreshToken(); // Save to EEPROM
    }
    else
    {
        return false;
    }

    return true;
    
}

/**
 * @brief Generate new auth token using refresh token.
 * 
 * @return true Generated new token successfully.
 * @return false Failed to generate new token.
 */
bool AuthManager::authWithRefreshToken()
{
    HTTPClient http;
    http.begin("https://oauth2.googleapis.com/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    std::string postData = "client_id=" + clientId + "&client_secret=" + clientSecret + "&refresh_token=" + refreshToken + "&grant_type=refresh_token";
    
    // Get response and code
    int httpResponseCode = http.POST(postData.c_str());
    String responseString = http.getString();

    printHttpResp(httpResponseCode, responseString);

    if (httpResponseCode == HTTP_CODE_OK) {

        Serial.println("\33[0;32mAuthenticated using refresh token.\33[0m");

        JsonDocument response;
        deserializeJson(response, responseString);
        http.end();

        // Save access tokens
        if(response["access_token"].is<const char *>() &&
           response["expires_in"].is<uint16_t>())
        {
            strcpy(accessToken, response["access_token"].as<const char *>());
            tokenLifetime = response["expires_in"];
        }
        else
        {
            return false;
        }

        return true;
    } else {
        Serial.println("\33[0;31mFailed to authenticate using refresh token. HTTP Response Code: " + String(httpResponseCode) + "\33[0m");

        return false;
    }
}

/**
 * @brief Save refresh token to persistent memory.
 * @todo - better memory management
 */
void AuthManager::saveRefreshToken()
{
    EEPROM.begin(EEPROM_SIZE);
    for (int i = 0; i < strlen(refreshToken); i++) {
        EEPROM.write(REFRESH_TOKEN_ADDRESS + i, refreshToken[i]);
    }
    EEPROM.write(REFRESH_TOKEN_ADDRESS + strlen(refreshToken), '\0'); // Null-terminate the string
    EEPROM.commit();
}

/**
 * @brief Read refresh token from persistent memory.
 * 
 */
void AuthManager::readRefreshToken()
{
    EEPROM.begin(EEPROM_SIZE);
    // std::string token = "";
    char token[MAX_TOKEN_LEN];
    memset(token, 0, sizeof(token)/sizeof(token[0]));
    char ch;
    for (int i = 0; i < MAX_TOKEN_LEN; i++) {
        ch = EEPROM.read(REFRESH_TOKEN_ADDRESS + i);
        if (ch == '\0') break; // Stop reading at null terminator
        token[i] = ch;
    }
    token[MAX_TOKEN_LEN-1] = '\0';
    // refreshToken = token;
    Serial.printf("Token: %s\n", token);
    strcpy(refreshToken, token);
}
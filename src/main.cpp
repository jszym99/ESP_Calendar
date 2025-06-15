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
WiFiServer server(80);

String html = "<!DOCTYPE html> \
<html lang=\"en\"> \
<head> \
    <meta charset=\"UTF-8\"> \
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
    <title>Google OAuth Redirect</title> \
    <script> \
        function redirectToGoogleOAuth() { \
            const clientId = document.getElementById('clientIdInput').value; // Get clientId from the text field \n \
            const redirectUri = \"http://espcal\"; // Replace with your valid redirect URI \n \
            const scope = \"https://www.googleapis.com/auth/userinfo.profile\"; \n \
            const responseType = \"code\"; // Use \"code\" for authorization code flow \n \
            const googleOAuthUrl = `https://accounts.google.com/o/oauth2/v2/auth?client_id=${clientId}&redirect_uri=${redirectUri}&response_type=${responseType}&scope=${scope}`; \
            window.open(googleOAuthUrl, '_blank'); \
        } \
    </script> \
</head> \
<body> \
    <h1>Google OAuth Redirect</h1> \
    <label for=\"clientIdInput\">Enter Client ID:</label> \
    <input type=\"text\" id=\"clientIdInput\" placeholder=\"Enter your Client ID\" /> \
    <button onclick=\"redirectToGoogleOAuth()\">Redirect to Google OAuth</button> \
</body> \
</html>";


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

	WiFiClient client = server.available(); // Check for incoming clients
    if (client)
    {
		String request = client.readStringUntil('\r'); // Read the request
        client.flush();

		client.println(html);

		client.stop(); // Close the connection
	}
}
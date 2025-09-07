#include "cal_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "utils/utils.h"

static HTTPClient http;

CalManager::CalManager(String (*func)())
{
    tokenFunction = func;
}

CalManager::~CalManager()
{
}

/**
 * @brief Fetch events from google calendar.
 * 
 * Note: hard coded primary calendar for now
 */
void CalManager::fetchEvents()
{
    JsonDocument response;
    String response_string;
    eventData tmp;
    String accessToken = tokenFunction(); // TODO: fallback on empty string
    
    const String calendarId = "primary";
    const String fields = "fields=items(id,summary,start,end)";

    // Convert current unix time to ISO 8601
    time_t currentTime = now();
    tm *tm = gmtime(&currentTime);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    String formattedTime = String(buf);


    // Construct URL
    const String URL = "https://www.googleapis.com/calendar/v3/calendars/" +
        calendarId + "/events?maxResults=" + String(eventCount) +
        "&orderBy=startTime&singleEvents=true&timeMin=" + formattedTime +
        "&" + fields;

	http.begin(URL);
    http.addHeader("Authorization",  "Bearer " + accessToken);

    // TODO: Fallback on request error
    int httpResponseCode = http.GET();
    response_string = http.getString();
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Response: " + response_string);

    deserializeJson(response, response_string);

    // Clear event data
    events = {};

    // Check for keys and save events info
    for(int i = 0; i < response["items"].size(); i++)
    {
        tmp = {};
        if(response["items"][i]["id"].is<String>())
        {
            tmp.eventId = response["items"][i]["id"].as<String>();
        }

        if(response["items"][i]["summary"].is<String>())
        {
            tmp.summary = response["items"][i]["summary"].as<String>();
        }

        // Save event start date (and time)
        if(response["items"][i]["start"]["date"].is<String>())
        {
            tmp.startDate = datetimeToSeconds(response["items"][i]["start"]["date"].as<String>());
            tmp.isAllDay = true;
        }
        else if (response["items"][i]["start"]["dateTime"].is<String>())
        {
            tmp.startDate = datetimeToSeconds(response["items"][i]["start"]["dateTime"].as<String>());
            tmp.isAllDay = false;
        }

        // Save event end date (and time)
        if(response["items"][i]["end"]["date"].is<String>())
        {
            tmp.endDate = datetimeToSeconds(response["items"][i]["end"]["date"].as<String>());
            tmp.isAllDay = true;
        }
        else if (response["items"][i]["end"]["dateTime"].is<String>())
        {
            tmp.endDate = datetimeToSeconds(response["items"][i]["end"]["dateTime"].as<String>());
            tmp.isAllDay = false;
        }

        events.push_back(tmp);
    }
}
#include "cal_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "utils/utils.h"

static HTTPClient http;

CalManager::CalManager(DisplayManager *displayManager)
{
    display = displayManager;
}

CalManager::~CalManager()
{
}

/**
 * @brief Fetch events from google calendar.
 * 
 * Note: hard coded primary calendar for now
 */
void CalManager::fetchEvents(String accessToken)
{
    JsonDocument response;
    String response_string;
    eventData newEvent;
    std::vector<eventData> newEvents = {};
    
    const String calendarId = "primary";
    const String fields = "fields=items(id,summary,start,end)";

    // Convert current unix time to ISO 8601
    time_t currentTime = now() - 7200; // Compensate time zone
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

    // Check for keys and save events info
    for(int i = 0; i < response["items"].size(); i++)
    {
        newEvent = {};
        if(response["items"][i]["id"].is<std::string>())
        {
            newEvent.eventId = response["items"][i]["id"].as<std::string>();
        }

        if(response["items"][i]["summary"].is<std::string>())
        {
            newEvent.summary = response["items"][i]["summary"].as<std::string>();
        }

        // Save event start date (and time)
        if(response["items"][i]["start"]["date"].is<String>())
        {
            newEvent.startDate = datetimeToSeconds(response["items"][i]["start"]["date"].as<String>());
            newEvent.isAllDay = true;
        }
        else if (response["items"][i]["start"]["dateTime"].is<String>())
        {
            newEvent.startDate = datetimeToSeconds(response["items"][i]["start"]["dateTime"].as<String>());
            newEvent.isAllDay = false;
        }

        // Save event end date (and time)
        if(response["items"][i]["end"]["date"].is<String>())
        {
            newEvent.endDate = datetimeToSeconds(response["items"][i]["end"]["date"].as<String>());
            newEvent.isAllDay = true;
        }
        else if (response["items"][i]["end"]["dateTime"].is<String>())
        {
            newEvent.endDate = datetimeToSeconds(response["items"][i]["end"]["dateTime"].as<String>());
            newEvent.isAllDay = false;
        }

        newEvents.push_back(newEvent);
    }

    if(newEvents != events & !newEvents.empty())
    {
        pendingUpdate = true;
        events = newEvents;
    }
}

void CalManager::showEvents()
{
    if(pendingUpdate)
    {
        std::string startTime;
        std::string endTime;
        std::string time;
        std::string date;

        display->clearItems();

        for(const auto &ev : events)
        {
            // Create date and time strings
            if(ev.startDate/SECS_PER_DAY == ev.endDate/SECS_PER_DAY)
            {
                startTime = timeToString(ev.startDate);
                endTime = timeToString(ev.endDate);
            }
            else
            {
                startTime = dateToString(ev.startDate);
                endTime = dateToString(ev.endDate);
            }

            time = (ev.isAllDay ? "All day" : startTime + " - " + endTime);

            // Create date sring and append "today" if needed
            date = (date != datetimeToString(ev.startDate, false, true) ? datetimeToString(ev.startDate, false, true) : "");
            if(now()/SECS_PER_DAY == ev.startDate/SECS_PER_DAY)
            {
                date += ", Today";
            }

            display->addItem(ev.summary, date, time);
        }
        display->dsiplay();
        pendingUpdate = false;
    }
}
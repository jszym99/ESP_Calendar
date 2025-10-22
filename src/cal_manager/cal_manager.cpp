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

void CalManager::fetchCalendarList(const String accessToken)
{
    JsonDocument response;
    String responseString;

    const char fields[] = "fields=items(id,summary,description,primary)";

    // Construct URL
    const char URL[] = "https://www.googleapis.com/calendar/v3/users/me/calendarList?fields=items(id,summary,description,primary)";
 
    http.begin(URL);
    http.addHeader("Authorization",  "Bearer " + accessToken);

    // Get response and code
    int httpResponseCode = http.GET();
    responseString = http.getString();
    
    printHttpResp(httpResponseCode, responseString);
    
    // TODO: Fallback on request error
    if(httpResponseCode != HTTP_CODE_OK)
    {
        http.end();
        return;
    }


    http.end();

    deserializeJson(response, responseString);

    Calendar calendar = {};
    std::vector<Calendar> newCalendarList = {};
    
    for(int i = 0; i < response["items"].size(); i++)
    {
        calendar.calendarId = response["items"][i]["id"].as<std::string>();

        calendar.summary = response["items"][i]["summary"].as<std::string>();
        calendar.description = response["items"][i]["description"].as<std::string>();
        if(response["items"][i]["primary"].is<bool>() & response["items"][i]["primary"].as<bool>() == true)
        {
            calendar.primary = true;
        }
        
        newCalendarList.push_back(calendar);
    }

    calendarList = updateVector(calendarList, newCalendarList);
}

std::vector<EventData> CalManager::listCalendarEvents(const String accessToken, std::string calendarId)
{
    JsonDocument response;
    String responseString;
    EventData newEvent;
    std::vector<EventData> newEvents = {};
    
    const std::string fields = "fields=items(id,summary,start,end)";

    // Convert current unix time to ISO 8601
    time_t currentTime = now() - 7200; // Compensate time zone
    tm *tm = gmtime(&currentTime);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    // String formattedTime = String(buf);

    // Replace # with character code (#23)
    while(calendarId.find("#") != std::string::npos)
    {
        calendarId.replace(calendarId.find("#"), 1, "%23");
    }

    // Construct URL
    const std::string URL = "https://www.googleapis.com/calendar/v3/calendars/" +
        calendarId + "/events?maxResults=" + std::to_string(eventCount) +
        "&orderBy=startTime&singleEvents=true&timeMin=" + buf +
        "&" + fields;

    // const std::string params = "/events?maxResults=" + std::to_string(eventCount)+ ;

	http.begin(URL.c_str());
    http.addHeader("Authorization",  "Bearer " + accessToken);

    Serial.println("Request: " + String(URL.c_str()));

    // Get response and code
    int httpResponseCode = http.GET();
    responseString = http.getString();
    
    printHttpResp(httpResponseCode, responseString);
    
    // TODO: Fallback on request error
    if(httpResponseCode != HTTP_CODE_OK)
    {
        http.end();
        return newEvents;
    }

    http.end();

    deserializeJson(response, responseString);

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

    return newEvents;
}

/**
 * @brief Fetch events from google calendar.
 * 
 * Note: hard coded primary calendar for now
 */
void CalManager::fetchEvents(const String accessToken)
{
    std::vector<EventData> newEvents = {};
    
    for(Calendar cal : calendarList)
    {
        if(cal.enabled)
        {
            std::vector<EventData> calendarEvents = listCalendarEvents(accessToken, cal.calendarId);

            if(!calendarEvents.empty())
            {
                newEvents.insert(newEvents.end(), calendarEvents.begin(), calendarEvents.end());
    
                std::sort(newEvents.begin(), newEvents.end());
    
                // Remove unneeded events
                if(newEvents.size() > eventCount)
                {
                    newEvents.erase(newEvents.begin() + eventCount, newEvents.end());
                }
            }

        }
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
        display->display();
        pendingUpdate = false;
    }
}
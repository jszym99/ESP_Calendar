#include "cal_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "utils/utils.h"

static HTTPClient http;

CalManager::CalManager(String (*func)())
{
    accessTokenFunc = func;
}

CalManager::~CalManager() 
{
}

void CalManager::fetchCalendarList()
{
    JsonDocument response;
    String responseString;

    const char fields[] = "fields=items(id,summary,description,primary)";

    // Construct URL
    const char URL[] = "https://www.googleapis.com/calendar/v3/users/me/calendarList?fields=items(id,summary,description,primary)";
 
    http.begin(URL);
    http.addHeader("Authorization",  "Bearer " + accessTokenFunc());

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

    // Calendar calendar = {};
    std::string calendarId, summary, description;
    bool primary;
    std::vector<Calendar> newCalendarList = {};
    
    for(int i = 0; i < response["items"].size(); i++)
    {
        calendarId = "";
        summary = "";
        description = "";
        primary = false;
        // calendar = {};
        calendarId = response["items"][i]["id"].as<std::string>();

        summary = response["items"][i]["summary"].as<std::string>();
        if(response["items"][i]["description"].is<std::string>())
        {
            description = response["items"][i]["description"].as<std::string>();
        }
        if(response["items"][i]["primary"].is<bool>() && response["items"][i]["primary"].as<bool>() == true)
        {
            primary = true;
        }

        newCalendarList.push_back(Calendar(calendarId, summary, description, primary));
    }

    calendarList = updateVector(calendarList, newCalendarList);
}

std::vector<CalendarEvent> CalManager::listCalendarEvents(std::string calendarId)
{
    JsonDocument response;
    String responseString;
    std::vector<CalendarEvent> newEvents = {};
    
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
    http.addHeader("Authorization",  "Bearer " + accessTokenFunc());

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
        std::string eventId, summary;
        time_t start, end;
        bool allDay;

        if(response["items"][i]["id"].is<std::string>())
        {
            eventId = response["items"][i]["id"].as<std::string>();
        }

        if(response["items"][i]["summary"].is<std::string>())
        {
            summary = response["items"][i]["summary"].as<std::string>();
        }

        // Save event start date (and time)
        if(response["items"][i]["start"]["date"].is<String>())
        {
            start = datetimeToSeconds(response["items"][i]["start"]["date"].as<String>());
            allDay = true;
        }
        else if (response["items"][i]["start"]["dateTime"].is<String>())
        {
            start = datetimeToSeconds(response["items"][i]["start"]["dateTime"].as<String>());
            allDay = false;
        }

        // Save event end date (and time)
        if(response["items"][i]["end"]["date"].is<String>())
        {
            end = datetimeToSeconds(response["items"][i]["end"]["date"].as<String>());
            allDay = true;
        }
        else if (response["items"][i]["end"]["dateTime"].is<String>())
        {
            end = datetimeToSeconds(response["items"][i]["end"]["dateTime"].as<String>());
            allDay = false;
        }

        newEvents.push_back(CalendarEvent(eventId, summary, start, end, allDay));
    }

    return newEvents;
}

/**
 * @brief Fetch events from google calendar.
 * 
 * Note: hard coded primary calendar for now
 */
void CalManager::fetchEvents()
{
    std::vector<CalendarEvent> newEvents = {};
    
    for(Calendar cal : calendarList)
    {
        if(cal.isEnabled())
        {
            std::vector<CalendarEvent> calendarEvents = listCalendarEvents(cal.getId());

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

        DisplayManager::clearItems();

        for(CalendarEvent ev : events)
        {
            DisplayManager::addItem(ev.getSummary(), ev.getDateStr(), ev.getDurationStr());
        }
        DisplayManager::display();
        pendingUpdate = false;
    }
}

const std::vector<Calendar>& CalManager::getCalendarList() const
{
    return calendarList;
}

const std::vector<Calendar*> CalManager::getCalendarPtrList()
{
    std::vector<Calendar*> out;
    
    for(Calendar &cal : calendarList)
    {
        out.push_back(&cal);
    }
    return out;
}
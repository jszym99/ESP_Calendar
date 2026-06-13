#include "cal_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "utils/utils.h"

static HTTPClient http;

CalManager::CalManager(AuthManager *authenticator)
{
    auth = authenticator;
    events.reserve(maxEventCount);
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
    http.addHeader("Authorization",  "Bearer " + String(auth->getAccessToken().c_str()));

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
    static char calendarId[BUFSIZ];
    static char summary[BUFSIZ];
    static char description[BUFSIZ];
    memset(calendarId, 0, sizeof(calendarId)/sizeof(calendarId[0]));
    memset(summary, 0, sizeof(summary)/sizeof(summary[0]));
    memset(description, 0, sizeof(description)/sizeof(description[0]));


    bool primary;
    // std::vector<Calendar> newCalendarVector = {};
    // Calendar newCalendar;
    static Calendar newCalendarList[maxCalendarCount];
    uint8_t newCalendarListSize{0};
    
    for(int i = 0; i < response["items"].size(); i++)
    {
        primary = false;

        strcpy(calendarId, response["items"][i]["id"].as<const char *>());

        strcpy(summary, response["items"][i]["summary"].as<const char *>());
        if(response["items"][i]["description"].is<const char *>())
        {
            strcpy(description, response["items"][i]["description"].as<const char *>());
        }
        if(response["items"][i]["primary"].is<bool>() && response["items"][i]["primary"].as<bool>() == true)
        {
            primary = true;
        }

        // newCalendarVector.push_back(Calendar(calendarId, summary, description, primary));
        if(newCalendarListSize < maxCalendarCount)
        {
            newCalendarList[newCalendarListSize++] = Calendar(calendarId, summary, description, primary);
        }
    }

    // Synchronize calendar enabled/disabled flag
    for(uint8_t oldCalNum; oldCalNum < calendarListSize; oldCalNum++)
    {
        // Check if calendar is in new list
        for(uint8_t newCalNum; newCalNum < newCalendarListSize; newCalNum++)
        {
            // If present update enable/disable flag
            if(calendarList[oldCalNum] == newCalendarList[newCalNum])
            {
                if(calendarList[oldCalNum].isEnabled())
                {
                    newCalendarList[newCalNum].enable();
                }
                else
                {
                    newCalendarList[newCalNum].disable();
                }

                break; // Calendar id's cannot repeat, no need to keep going
            }
        }
    }

    // Update list length
    calendarListSize = newCalendarListSize;
    
    // Copy new calendar data to the object list
    for(uint8_t newCalNum; newCalNum < newCalendarListSize; newCalNum++)
    {
        calendarList[newCalNum] = newCalendarList[newCalNum];
    }

    // calendars = updateVector(calendars, newCalendarVector);
}

std::vector<CalendarEvent> CalManager::listCalendarEvents(std::string calendarId)
{
    JsonDocument response;
    String responseString;
    std::vector<CalendarEvent> newEvents = {};
    
    const char fields[] = "fields=items(id,summary,start,end)";

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
        calendarId + "/events?maxResults=" + std::to_string(maxEventCount) +
        "&orderBy=startTime&singleEvents=true&timeMin=" + buf +
        "&" + fields;

    // const std::string params = "/events?maxResults=" + std::to_string(maxEventCount)+ ;

	http.begin(URL.c_str());
    http.addHeader("Authorization",  "Bearer " + String(auth->getAccessToken().c_str()));

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
        // std::string eventId, summary;
        static char eventId[BUFSIZ];
        static char summary[BUFSIZ];

        memset(eventId, 0, sizeof(eventId)/sizeof(eventId[0]));
        memset(summary, 0, sizeof(summary)/sizeof(summary[0]));

        time_t start, end;
        bool allDay;

        if(response["items"][i]["id"].is<const char *>())
        {
            // eventId = response["items"][i]["id"].as<std::string>();
            strcpy(eventId, response["items"][i]["id"].as<const char *>());
        }

        if(response["items"][i]["summary"].is<const char *>())
        {
            // summary = response["items"][i]["summary"].as<std::string>();
            strcpy(summary, response["items"][i]["summary"].as<const char *>());
        }

        // Save event start date (and time)
        if(response["items"][i]["start"]["date"].is<const char *>())
        {
            start = datetimeToSeconds(response["items"][i]["start"]["date"].as<const char *>());
            allDay = true;
        }
        else if (response["items"][i]["start"]["dateTime"].is<const char *>())
        {
            start = datetimeToSeconds(response["items"][i]["start"]["dateTime"].as<const char *>());
            allDay = false;
        }

        // Save event end date (and time)
        if(response["items"][i]["end"]["date"].is<const char *>())
        {
            end = datetimeToSeconds(response["items"][i]["end"]["date"].as<const char *>());
            allDay = true;
        }
        else if (response["items"][i]["end"]["dateTime"].is<const char *>())
        {
            end = datetimeToSeconds(response["items"][i]["end"]["dateTime"].as<const char *>());
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
    
    // Fetch events for every enabled calendar
    for(uint8_t calNum = 0; calNum < calendarListSize; calNum++)
    {
        if(calendarList[calNum].isEnabled())
        {
            std::vector<CalendarEvent> calendarEvents = listCalendarEvents(calendarList[calNum].getId());

            // Check if calendar has any events
            if(!calendarEvents.empty())
            {
                newEvents.insert(newEvents.end(), calendarEvents.begin(), calendarEvents.end());
    
                std::sort(newEvents.begin(), newEvents.end());
    
                // Remove unneeded events
                if(newEvents.size() > maxEventCount)
                {
                    newEvents.erase(newEvents.begin() + maxEventCount, newEvents.end());
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

const std::vector<Calendar>& CalManager::getCalendarList() const
{
    return calendars;
}

const std::vector<Calendar*> CalManager::getCalendarPtrList()
{
    std::vector<Calendar*> out;
    
    for(Calendar &cal : calendars)
    {
        out.push_back(&cal);
    }
    return out;
}

bool CalManager::isPendingUpdate()
{
    bool out = pendingUpdate;
    pendingUpdate = false;
    return out;
}
#include "cal_manager.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <StreamUtils.h>
#include "utils/utils.h"

static WiFiClientSecure client;
static HTTPClient http;

/**
 * @brief Merges and sorts (ascending) two lists of the same size and type
 * 
 * @param list Input/output list
 * @param newList Input list
 * @param listLen List length
 * @param newListLen New list length
 * @return uint8_t 
 */
uint8_t mergeAndSort(CalendarEvent *list, const CalendarEvent *newList, uint8_t listLen, uint8_t newListLen)
{
    CalendarEvent out[maxEventCount];

    uint8_t l1 = 0;
    uint8_t l2 = 0;
    uint8_t i = 0;

    while((i < maxEventCount) && (i < (listLen + newListLen)))
    {
        // No more items in the first list
        if(l1 >= listLen)
        {
            out[i] = newList[l2];
            l2++;
        }
        // No more items in the second list
        else if(l2 >= newListLen)
        {
            out[i] = list[l1];
            l1++;
        }
        // Pick first item from either list
        else if(list[l1] < newList[l2])
        {
            out[i] = list[l1];
            l1++;
        }
        else
        {
            out[i] = newList[l2];
            l2++;
        }
        i++;

        // Serial.println(out[i].getSummary());
    }

    memcpy(list, out, i * sizeof(CalendarEvent));

    return i;
}

DeserializationError fetchJson(const char *url, JsonDocument &doc, const char *bearerToken = nullptr)
{
    // WiFiClientSecure client;
    // HTTPClient http;

    client.setInsecure();

    // Tell HTTPClient to collect the Transfer-Encoding header
    const char* keys[] = {"Transfer-Encoding"};
    http.collectHeaders(keys, 1);

    // Initialize the connection
    if (!http.begin(client, url)) {
        Serial.println("Error: Failed to initialize HTTP connection.");
        return DeserializationError::EmptyInput;
    }

    // Inject the Auth Header (if a token was provided)
    if (bearerToken != nullptr) {
        char authHeader[512];
        snprintf(authHeader, sizeof(authHeader), "Bearer %s", bearerToken);
        http.addHeader("Authorization", authHeader);
    }
    
    // Execute the request
    int httpCode = http.GET();

    // Handle HTTP errors
    if (httpCode != HTTP_CODE_OK) { 
        Serial.printf("HTTP GET Error Code: %d\n", httpCode);
        
        if (httpCode > 0) {
        Serial.println("Server Response: " + http.getString()); 
        }
        
        http.end();
        return DeserializationError::EmptyInput; 
    }

    // Get the raw and the decoded stream
    Stream& rawStream = http.getStream();
    
    // Check if the server used chunked encoding
    bool isChunked = http.header("Transfer-Encoding").equalsIgnoreCase("chunked");
    
    DeserializationError error;

    // Choose the right stream depending on the Transfer-Encoding header
    if (isChunked) {
        // Strip the hex chunks
        ChunkDecodingStream decodedStream(rawStream);
        // Wiretap the clean stream
        ReadLoggingStream loggingStream(decodedStream, Serial);
        // Parse the wiretap
        error = deserializeJson(doc, loggingStream);
    } else {
        // Wiretap the raw stream directly
        ReadLoggingStream loggingStream(rawStream, Serial);
        // Parse the wiretap
        error = deserializeJson(doc, loggingStream);
    }

    // Close the connection (and free socket memory)
    http.end();

    return error;
}


CalManager::CalManager(AuthManager *authenticator)
{
    auth = authenticator;
}

CalManager::~CalManager() 
{
}

void CalManager::fetchCalendarList()
{
    JsonDocument response;
    // String responseString;

    // const char fields[] = "fields=items(id,summary,description,primary)";

    // // Construct URL
    const char URL[] = "https://www.googleapis.com/calendar/v3/users/me/calendarList?fields=items(id,summary,description,primary)";

    fetchJson(URL, response, auth->getAccessToken());

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
    for(uint8_t oldCalNum = 0; oldCalNum < calendarListSize; oldCalNum++)
    {
        // Check if calendar is in new list
        for(uint8_t newCalNum = 0; newCalNum < newCalendarListSize; newCalNum++)
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
    for(uint8_t newCalNum = 0; newCalNum < newCalendarListSize; newCalNum++)
    {
        calendarList[newCalNum] = newCalendarList[newCalNum];
    }

    // calendars = updateVector(calendars, newCalendarVector);
}

uint8_t CalManager::listCalendarEvents(CalendarEvent *events, const char* calendarId)
{
    JsonDocument response;
    String responseString;
    
    const char fields[] = "fields=items(id,summary,start,end)";

    // Convert current unix time to ISO 8601
    time_t currentTime = now() - 7200; // Compensate time zone
    tm *tm = gmtime(&currentTime);
    char buf[25];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", tm);
    // String formattedTime = String(buf);

    std::string encodedId = (calendarId != nullptr) ? calendarId : "";

    // Replace # with character code (#23)
    while(encodedId.find("#") != std::string::npos)
    {
        encodedId.replace(encodedId.find("#"), 1, "%23");
    }

    // Construct URL
    const std::string URL = "https://www.googleapis.com/calendar/v3/calendars/" +
        encodedId + "/events?maxResults=" + std::to_string(maxEventCount) +
        "&orderBy=startTime&singleEvents=true&timeMin=" + buf +
        "&" + fields;

    fetchJson(URL.c_str(), response, auth->getAccessToken());

    // Check for keys and save events info
    uint8_t i;
    for(i = 0; (i < response["items"].size()) &&  (i < maxEventCount); i++)
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

        events[i] = CalendarEvent(eventId, summary, start, end, allDay);
    }

    return i;
}

/**
 * @brief Fetch events from google calendar.
 * 
 * Note: hard coded primary calendar for now
 */
void CalManager::fetchEvents()
{
    CalendarEvent newEvents[maxEventCount];
    uint8_t newEventCount = 0;
    
    // Fetch events for every enabled calendar
    for(uint8_t calNum = 0; calNum < calendarListSize; calNum++)
    {
        if(calendarList[calNum].isEnabled())
        {
            CalendarEvent calendarEvents[maxEventCount];
            uint8_t eventCount = listCalendarEvents(calendarEvents, calendarList[calNum].getId());
            
            // Check if calendar has any events
            if(eventCount != 0)
            {
                newEventCount = mergeAndSort(newEvents, calendarEvents, newEventCount, eventCount);
            }
        }
    }

    // New event list is not empty
    if(newEventCount != 0)
    {
        // Check and update each element if needed
        for(uint8_t i = 0; i < newEventCount; i++)
        {
            if(!(newEvents[i] == eventList[i]))
            {
                eventList[i] = newEvents[i];
                pendingUpdate = true;
            }
        }
        eventListSize = newEventCount;
    }
}

// const std::vector<Calendar>& CalManager::getCalendarList() const
// {
//     return calendars;
// }

// const std::vector<Calendar*> CalManager::getCalendarPtrList()
// {
//     std::vector<Calendar*> out;
    
//     for(Calendar &cal : calendars)
//     {
//         out.push_back(&cal);
//     }
//     return out;
// }

bool CalManager::isPendingUpdate()
{
    bool out = pendingUpdate;
    pendingUpdate = false;
    return out;
}

uint8_t CalManager::getCalendarList(Calendar *buf, uint8_t bufSize) const
{
    uint8_t count = calendarListSize < bufSize ? calendarListSize : bufSize;
    for (uint8_t i = 0; i < count; i++) {
        buf[i] = calendarList[i];
    }
    return count;
}

uint8_t CalManager::getCalendarPtrList(Calendar **buf, uint8_t bufSize)
{
    uint8_t count = calendarListSize < bufSize ? calendarListSize : bufSize;
    for (uint8_t i = 0; i < count; i++) {
        buf[i] = &calendarList[i];
    }
    return count;
}

#ifndef CAL_MANAGER_H
#define CAL_MANAGER_H
#include <Arduino.h>
#include <TimeLib.h>
#include <vector>
#include "auth_manager/auth_manager.h"
#include "display_manager/display_manager.h"

constexpr uint8_t eventCount = 3;

struct Calendar
{
    String calendarId;
    String summary;
    String description;
    bool enabled = true;
    bool primary = false;

    bool operator==(const Calendar& other) const
    {
        return calendarId == other.calendarId;
    }
};

struct EventData
{
    std::string eventId;
    std::string summary;
    time_t startDate;
    time_t endDate;
    bool isAllDay;

    // Overload == operator
    bool operator==(const EventData& other) const
    {
        return eventId == other.eventId &&
               summary == other.summary &&
               startDate == other.startDate &&
               endDate == other.endDate &&
               isAllDay == other.isAllDay;
    }

    bool operator< (const EventData& other) const
    {
        return startDate < other.startDate;
    }
};

class CalManager
{
private:
    std::vector<EventData> events = {};
    std::vector<Calendar> calendarList = {};
    bool pendingUpdate = false;
    DisplayManager *display;
public:
    /**
     * @brief Construct a new Cal Manager object.
     * 
     * @param func Access token getter function.
     */
    CalManager(DisplayManager *displayManager);

    /**
     * @brief Destroy the Cal Manager object.
     * 
     */
    ~CalManager();

    void fetchCalendarList(const String accessToken);

    /**
     * @brief Fetch events from google calendar.
     * 
     * @note Hard coded primary calendar for now.
     */
    void fetchEvents(const String accessToken);
    std::vector<EventData> listCalendarEvents(const String accessToken, const String calendarId = "primary");

    void showEvents();
};

#endif /* CAL_MANAGER_H */
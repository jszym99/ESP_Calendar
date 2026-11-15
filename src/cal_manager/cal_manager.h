#ifndef CAL_MANAGER_H
#define CAL_MANAGER_H
#include <Arduino.h>
#include <TimeLib.h>
#include "auth_manager/auth_manager.h"
// #include "display_manager/display_manager.h"
#include "calendar.h"
#include "calendar_event.h"

constexpr uint8_t maxEventCount{3u};
constexpr uint8_t maxCalendarCount{10u}; // Max number of calendars (10 should be big enough)

class CalManager
{
private:
    Calendar calendarList[maxCalendarCount];
    uint8_t calendarListSize = 0;
    CalendarEvent eventList[maxEventCount];
    uint8_t eventListSize = 0;
    AuthManager *auth;
    bool pendingUpdate = false;
public:
    /**
     * @brief Construct a new Cal Manager object.
     * 
     * @param func Access token getter function.
     */
    CalManager(AuthManager *authenticator);

    /**
     * @brief Destroy the Cal Manager object.
     * 
     */
    ~CalManager();

    void fetchCalendarList();

    /**
     * @brief Fetch events from google calendar.
     * 
     * @note Hard coded primary calendar for now.
     */
    void fetchEvents();
    uint8_t listCalendarEvents(CalendarEvent *events, const char* calendarId = "primary");

    const CalendarEvent *getEventsPtr() const {return eventList;}
    const uint8_t getEventCount() const {return eventListSize;}

    // Copy calendar list into caller-provided buffer. Returns number of items copied.
    uint8_t getCalendarList(Calendar *buf, uint8_t bufSize) const;
    // Copy calendar pointers into caller-provided buffer. Returns number of items copied.
    uint8_t getCalendarPtrList(Calendar **buf, uint8_t bufSize);

    bool isPendingUpdate();
};

#endif /* CAL_MANAGER_H */
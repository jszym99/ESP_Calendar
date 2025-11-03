#ifndef CAL_MANAGER_H
#define CAL_MANAGER_H
#include <Arduino.h>
#include <TimeLib.h>
#include <vector>
#include "auth_manager/auth_manager.h"
#include "display_manager/display_manager.h"
#include "calendar.h"
#include "calendar_event.h"

constexpr uint8_t eventCount = 3;

class CalManager
{
private:
    std::vector<CalendarEvent> events = {};
    std::vector<Calendar> calendarList = {};
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
    std::vector<CalendarEvent> listCalendarEvents(std::string calendarId = "primary");

    const std::vector<CalendarEvent> &getEvents() {return events;}

    // Return read-only reference to calendar list for external rendering
    const std::vector<Calendar>& getCalendarList() const;
    const std::vector<Calendar*> getCalendarPtrList();
};

#endif /* CAL_MANAGER_H */
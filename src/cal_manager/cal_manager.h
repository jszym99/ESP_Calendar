#ifndef CAL_MANAGER_H
#define CAL_MANAGER_H
#include <Arduino.h>
#include <TimeLib.h>
#include <vector>
#include "auth_manager/auth_manager.h"
#include "display_manager/display_manager.h"

constexpr uint8_t eventCount = 5;

struct eventData
{
    std::string eventId;
    std::string summary;
    time_t startDate;
    time_t endDate;
    bool isAllDay;

    // Overload == operator
    bool operator==(const eventData& other) const {
        return eventId == other.eventId &&
               summary == other.summary &&
               startDate == other.startDate &&
               endDate == other.endDate &&
               isAllDay == other.isAllDay;
    }
};

class CalManager
{
private:
    std::vector<eventData> events = {};
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

    /**
     * @brief Fetch events from google calendar.
     * 
     * @note Hard coded primary calendar for now.
     */
    void fetchEvents(String accessToken);

    void showEvents();
};

#endif /* CAL_MANAGER_H */
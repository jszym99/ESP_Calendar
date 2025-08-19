#ifndef CAL_MANAGER_H
#define CAL_MANAGER_H
#include <Arduino.h>
#include <TimeLib.h>
#include <vector>
#include "auth_manager/auth_manager.h"

constexpr uint8_t eventCount = 5;

struct eventData
{
    String summary;
    time_t startDate;
    time_t endDate;
    bool isAllDay;

};

class CalManager
{
private:
    std::vector<eventData> events;
    String (*tokenFunction)();
public:
    /**
     * @brief Construct a new Cal Manager object.
     * 
     * @param func Access token getter function.
     */
    CalManager(String (*func)());

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
    void fetchEvents();

    /**
     * @brief Get the Events object.
     * @note Events are sorted chronologically from current system time.
     * 
     * @return std::vector<eventData> 
     */
    std::vector<eventData> getEvents() {return events;} 
};

#endif /* CAL_MANAGER_H */
#include "calendar_event.h"
#include "utils/utils.h"

/**
 * @brief Construct a new Calendar Event:: Calendar Event object
 * 
 * @param id        Unique Id of the event
 * @param sum       Event summary (title)
 * @param start     Start time/date
 * @param end       End time/date
 * @param allDay    Is event all day
 */
CalendarEvent::CalendarEvent(std::string id, std::string sum, time_t start, time_t end, bool allDay)
{
    eventId = id;
    summary = sum;
    startDate = start;
    endDate = end;
    isAllDay = allDay;
}

/**
 * @brief Destroy the Calendar Event:: Calendar Event object
 * 
 */
CalendarEvent::~CalendarEvent()
{
}

/**
 * @brief Getter event start date in string format
 * 
 * @return std::string Event date (EEE, dd/MM)
 */
std::string CalendarEvent::getDateStr()
{
    std::string date = datetimeToString(startDate, false, true); 
    if(now()/SECS_PER_DAY == startDate/SECS_PER_DAY)
    {
        date += ", Today";
    }
    return date;
}

/**
 * @brief Getter for event duration in string format
 * 
 * @return std::string Event duration (All day/Start-End time/Start-End date and time)
 */
std::string CalendarEvent::getDurationStr()
{
    if(isAllDay)
    {
        return "All day";
    }
    else if (startDate/SECS_PER_DAY == endDate/SECS_PER_DAY) //Same day
    {
        return (timeToString(startDate) + " - " + timeToString(endDate));
    }
    else
    {
        return (dateToString(startDate) + " - " + dateToString(endDate));
    }
}
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

const char * CalendarEvent::getSummary() const
{
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0]));
    strcpy(out, summary.c_str());
    
    return out;
}

/**
 * @brief Getter event start date in string format
 * 
 * @return std::string Event date (EEE, dd/MM)
 */
const char * CalendarEvent::getDateStr() const
{
    static char date[BUFSIZ];
    memset(date, 0, sizeof(date)/sizeof(date[0]));
    // strcpy(date, datetimeToString(startDate, false, true).c_str());
    strcat(date, dayShortStr(weekday(startDate)));
    strcat(date, ", ");
    strcat(date, dateStr(startDate));

    if(now()/SECS_PER_DAY == startDate/SECS_PER_DAY)
    {
        strcat(date, ", Today");
    }
    return date;
}

/**
 * @brief Getter for event duration in string format
 * 
 * @return std::string Event duration (All day/Start-End time/Start-End date and time)
 */
const char * CalendarEvent::getDurationStr() const
{
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0]));
    if(isAllDay)
    {
        strcpy(out, "All day");
    }
    else if (startDate/SECS_PER_DAY == endDate/SECS_PER_DAY) //Same day
    {
        // strcpy(out, (timeToString(startDate) + " - " + timeToString(endDate)).c_str());
        strcat(out, timeStr(startDate));
        strcat(out, " - ");
        strcat(out, timeStr(endDate));
    }
    else
    {
        // strcpy(out, (dateToString(startDate) + " - " + dateToString(endDate)).c_str());
        strcat(out, dateStr(startDate));
        strcat(out, " - ");
        strcat(out, dateStr(endDate));
    }
    return out;
}
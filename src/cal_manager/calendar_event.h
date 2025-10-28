#ifndef CALENDAR_EVENT_H
#define CALENDAR_EVENT_H

#include <Arduino.h>

class CalendarEvent
{
private:
    std::string eventId;
    std::string summary;
    time_t startDate;
    time_t endDate;
    bool isAllDay;
public:
    CalendarEvent(std::string id, std::string sum, time_t start, time_t end, bool allDay);
    ~CalendarEvent();

    std::string getSummary() {return summary;}
    std::string getDateStr();
    std::string getDurationStr();

    // Overload == operator
    bool operator==(const CalendarEvent& other) const
    {
        return eventId == other.eventId &&
               summary == other.summary &&
               startDate == other.startDate &&
               endDate == other.endDate &&
               isAllDay == other.isAllDay;
    }

    bool operator< (const CalendarEvent& other) const
    {
        return startDate < other.startDate;
    }
};


#endif /* CALENDAR_EVENT_H */
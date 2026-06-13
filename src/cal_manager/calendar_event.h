#ifndef CALENDAR_EVENT_H
#define CALENDAR_EVENT_H

#include <Arduino.h>

class CalendarEvent
{
private:
    char eventId[BUFSIZ];
    char summary[BUFSIZ];
    time_t startDate;
    time_t endDate;
    bool isAllDay;
public:
    CalendarEvent();
    CalendarEvent(const char *id, const char *sum, time_t start, time_t end, bool allDay);
    ~CalendarEvent();

    const char * getSummary() const;
    const char * getDateStr() const;
    const char * getDurationStr() const;

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
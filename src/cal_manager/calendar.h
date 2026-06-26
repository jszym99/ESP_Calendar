#ifndef CALENDAR_H
#define CALENDAR_H
#include <Arduino.h>


class Calendar
{
private:
    char calendarId[BUFSIZ];
    char summary[BUFSIZ];
    char description[BUFSIZ];
    bool enabled = true;
    bool primary = false;
public:
    Calendar(const char *calId, const char *sum, const char *desc, bool en, bool prim);
    Calendar(const char *calId, const char *sum, const char *desc, bool prim);
    Calendar(const char *calId, const char *sum, const char *desc);
    Calendar();
    ~Calendar();

    void enable() {enabled = true;}
    void disable() {enabled = false;}

    const char * getId() const {return calendarId;}
    const char * getSummary() const {return summary;}
    const char * getDescription() const {return description;}
    bool isPrimary() {return primary;}
    bool isEnabled() {return enabled;}

    bool operator==(const Calendar& other) const
    {
        // return calendarId == other.calendarId;
        return strcmp(calendarId, other.calendarId);
    }
};

#endif /* CALENDAR_H */
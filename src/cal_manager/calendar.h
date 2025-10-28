#ifndef CALENDAR_H
#define CALENDAR_H
#include <Arduino.h>


class Calendar
{
private:
    std::string calendarId;
    std::string summary;
    std::string description;
    bool enabled = true;
    bool primary = false;
public:
    Calendar(std::string calId, std::string sum, std::string desc, bool en, bool prim);
    Calendar(std::string calId, std::string sum, std::string desc, bool prim);
    Calendar(std::string calId, std::string sum, std::string desc);
    ~Calendar();

    void enable() {enabled = true;}
    void disable() {enabled = false;}

    std::string getId() {return calendarId;}
    std::string getSummary() {return summary;}
    std::string getDescription() {return description;}
    bool isPrimary() {return primary;}
    bool isEnabled() {return enabled;}

    bool operator==(const Calendar& other) const
    {
        return calendarId == other.calendarId;
    }
};

#endif /* CALENDAR_H */
#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <TimeLib.h>
#include <vector>

/**
 * @brief Make human readable date and time from unix time
 * @param t time in seconds since 1st Jan 1970
 * @param b true if exclude time from date
 * 
 */
// #define DATETIME_STR(t, y, b) String(dayShortStr(weekday(t))) + ", " + \
//                        (day(t) < 10 ? "0" : "") + String(day(t)) + " " + \
//                        String(monthShortStr(month(t))) + " " + \
//                        (y ? (String(year(t)) + " ") : "") + \
//                        (!b ? (hour(t) < 10 ? "0" : "") + String(hour(t)) + ":" + \
//                        (minute(t) < 10 ? "0" : "") + String(minute(t)) : "")

/**
 * @brief Parses ISO 8601 to Unix time
 * 
 * @param datetime String containing date and time in ISO 8601 format.
 * @return time_t Time in Unix format (seconds since 1970).
 */
time_t datetimeToSeconds(const String& datetime);

inline std::string datetimeToString(time_t time, bool showYear = false, bool allDay = false, bool seconds = false)
{
    return (std::string(dayShortStr(weekday(time))) + ", " +
            (day(time) < 10 ? "0" : "") + std::to_string(day(time)) + " " + 
            std::string(monthShortStr(month(time))) + " " + 
            (showYear ? (std::to_string(year(time)) + " ") : "") + 
            (!allDay ? (hour(time) < 10 ? "0" : "") + std::to_string(hour(time)) + ":" + 
            (minute(time) < 10 ? "0" : "") + std::to_string(minute(time)) : "") + 
            (seconds ? (second(time) < 10 ? ":0" : ":") + std::to_string(second(time)) : ""));
    }

inline std::string timeToString(time_t time)
{
    return (hour(time) < 10 ? "0" : "") + std::to_string(hour(time)) + ":" + (minute(time) < 10 ? "0" : "") + std::to_string(minute(time));
}

inline std::string dateToString(time_t date)
{
    return (day(date) < 10 ? "0" : "") + std::to_string(day(date)) + " " +  std::string(monthShortStr(month(date))) + " " +  timeToString(date);
}

template <typename T>
inline std::vector<T> updateVector(std::vector<T> oldVector, std::vector<T> newVector)
{
    // Add new elements that are not in old vector
    for(auto vec : newVector)
    {
        if(std::find(oldVector.begin(), oldVector.end(), vec) == oldVector.end())
        {
            oldVector.push_back(vec);
        }
    }

    // Remove old elements that are not in new vector
    for(auto it = oldVector.begin(); it != oldVector.end(); )
    {
        if(std::find(newVector.begin(), newVector.end(), *it) == newVector.end())
        {
            it = oldVector.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return oldVector;
}

void printHttpResp(int code, String response);

#endif /* UTILS_H */
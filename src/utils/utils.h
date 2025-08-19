#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * @brief Make human readable date and time from unix time
 * @param t time in seconds since 1st Jan 1970
 * @param b true if exclude time from date
 * 
 */
#define DATETIME_STR(t, y, b) String(dayShortStr(weekday(t))) + ", " + \
                       (day(t) < 10 ? "0" : "") + String(day(t)) + " " + \
                       String(monthShortStr(month(t))) + " " + \
                       (y ? (String(year(t)) + " ") : "") + \
                       (!b ? (hour(t) < 10 ? "0" : "") + String(hour(t)) + ":" + \
                       (minute(t) < 10 ? "0" : "") + String(minute(t)) : "")

/**
 * @brief Parses ISO 8601 to Unix time
 * 
 * @param datetime String containing date and time in ISO 8601 format.
 * @return time_t Time in Unix format (seconds since 1970).
 */
time_t datetimeToSeconds(const String& datetime);

#endif /* UTILS_H */
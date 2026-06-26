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
time_t datetimeToSeconds(const char* datetime);

// inline std::string datetimeToString(time_t time, bool showYear = false, bool allDay = false, bool seconds = false)
// {
//     return (std::string(dayShortStr(weekday(time))) + ", " +
//             (day(time) < 10 ? "0" : "") + std::to_string(day(time)) + " " + 
//             std::string(monthShortStr(month(time))) + " " + 
//             (showYear ? (std::to_string(year(time)) + " ") : "") + 
//             (!allDay ? (hour(time) < 10 ? "0" : "") + std::to_string(hour(time)) + ":" + 
//             (minute(time) < 10 ? "0" : "") + std::to_string(minute(time)) : "") + 
//             (seconds ? (second(time) < 10 ? ":0" : ":") + std::to_string(second(time)) : ""));
//     }

// inline std::string timeToString(time_t time)
// {
//     return (hour(time) < 10 ? "0" : "") + std::to_string(hour(time)) + ":" + (minute(time) < 10 ? "0" : "") + std::to_string(minute(time));
// }

inline const char * timeStr(time_t time)
{
    static char buf[BUFSIZ];
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0])); // Clear buffer

    if(hour(time) < 10)
        strcat(out, "0");

    itoa(hour(time), buf, 10);
    strcat(out, buf);

    strcat(out, ":");

    if(minute(time) < 10)
        strcat(out, "0");

    itoa(minute(time), buf, 10);
    strcat(out, buf);

    return out;
}

inline const char * timeLongStr(time_t time)
{
    static char buf[BUFSIZ];
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0])); // Clear buffer

    strcat(out, timeStr(time));
    strcat(out, ":");

    if(second(time) < 10)
        strcat(out, "0");

    itoa(second(time), buf, 10);
    strcat(out, buf);

    return out;
}

// inline std::string dateToString(time_t date)
// {
//     return (day(date) < 10 ? "0" : "") + std::to_string(day(date)) + " " +  std::string(monthShortStr(month(date))) + " " +  timeToString(date);
// }

inline const char * dateStr(time_t date)
{
    static char buf[BUFSIZ];
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0])); // Clear buffer

    if(day(date) < 10)
        strcat(out, "0");

    itoa(day(date), buf, 10);
    strcat(out, buf);

    strcat(out, " ");
    strcat(out, monthShortStr(month(date)));

    // strcat(out, timeStr(date));

    return out;
}

inline const char * dateLongStr(time_t date)
{
    static char buf[BUFSIZ];
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0])); // Clear buffer

    strcat(out, dayShortStr(weekday(date)));
    strcat(out, ", ");

    strcat(out, dateStr(date));
    strcat(out, " ");

    itoa(year(date), buf, 10);
    strcat(out, buf);

    return out;
}

inline char * datetimeStr(time_t time)
{
    static char out[BUFSIZ];
    memset(out, 0, sizeof(out)/sizeof(out[0])); // Clear buffer

    strcat(out, dateLongStr(time));

    strcat(out, " ");

    strcat(out, timeLongStr(time));

    return out;
}

void printHttpResp(int code, String response);

#endif /* UTILS_H */
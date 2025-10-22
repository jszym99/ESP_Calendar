#include "utils.h"
#include <time.h>
#include <vector>


/**
 * @brief Parses ISO 8601 to Unix time
 * 
 * @param datetime String containing date and time in ISO 8601 format.
 * @return time_t Time in Unix format (seconds since 1970).
 */
time_t datetimeToSeconds(const String& datetime) {
    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
    int tIndex = datetime.indexOf('T');
    String datePart = datetime;
    String timePart = "";

    if (tIndex != -1) {
        datePart = datetime.substring(0, tIndex);
        timePart = datetime.substring(tIndex + 1);

        // Remove 'Z' or timezone offset
        int zIndex = timePart.indexOf('Z');
        if (zIndex != -1) timePart = timePart.substring(0, zIndex);
        int plusIndex = timePart.indexOf('+');
        int minusIndex = timePart.indexOf('-');
        int tzIndex = (plusIndex != -1) ? plusIndex : ((minusIndex > 0) ? minusIndex : -1);
        if (tzIndex != -1) timePart = timePart.substring(0, tzIndex);
        int dotIndex = timePart.indexOf('.');
        if (dotIndex != -1) timePart = timePart.substring(0, dotIndex);
    }

    // Parse date
    int firstDash = datePart.indexOf('-');
    int secondDash = datePart.indexOf('-', firstDash + 1);
    if (firstDash == -1 || secondDash == -1) return 0;
    year = datePart.substring(0, firstDash).toInt();
    month = datePart.substring(firstDash + 1, secondDash).toInt();
    day = datePart.substring(secondDash + 1).toInt();

    // Parse time if present
    if (timePart.length() > 0) {
        int firstColon = timePart.indexOf(':');
        int secondColon = timePart.indexOf(':', firstColon + 1);
        if (firstColon != -1) {
            hour = timePart.substring(0, firstColon).toInt();
            if (secondColon != -1) {
                minute = timePart.substring(firstColon + 1, secondColon).toInt();
                second = timePart.substring(secondColon + 1).toInt();
            } else {
                minute = timePart.substring(firstColon + 1).toInt();
            }
        }
    }

    struct tm tmTime;
    tmTime.tm_year = year - 1900;
    tmTime.tm_mon = month - 1;
    tmTime.tm_mday = day;
    tmTime.tm_hour = hour;
    tmTime.tm_min = minute;
    tmTime.tm_sec = second;
    tmTime.tm_isdst = -1;

    return mktime(&tmTime);
}

void printHttpResp(int code, String response)
{

    Serial.print("HTTP Response Code: ");

    switch (code/100)
    {
    case 1:
    case 2:
        Serial.print("\33[0;32m");
        break;
    case 3:
    case 4:
    case 5:
        Serial.print("\33[0;31m");
        break;
    default:
        Serial.print("\33[0m");
        break;
    }

    Serial.println(code);
    Serial.print("\33[0m");

    Serial.println("Response: " + response);
}

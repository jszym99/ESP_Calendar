#include "utils.h"
#include <time.h>
#include <vector>


/**
 * @brief Parses ISO 8601 to Unix time without dynamic memory allocation
 * * @param datetime Null-terminated char array containing date and time in ISO 8601 format.
 * @return time_t Time in Unix format (seconds since 1970).
 */
time_t datetimeToSeconds(const char* datetime) {
    // Prevent null pointer crashes
    if (datetime == nullptr) return 0;

    int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;

    // sscanf reads the string according to the format. 
    // It will safely stop parsing if it hits the end of the string early, 
    // or if it encounters unmapped characters like 'Z', '.', '+', or '-'.
    int parsed = sscanf(datetime, "%d-%d-%dT%d:%d:%d", 
                        &year, &month, &day, &hour, &minute, &second);

    // We require at least the Year, Month, and Day to be parsed successfully
    if (parsed < 3) return 0;

    struct tm tmTime = {0}; // Initialize to zero to clear garbage memory
    tmTime.tm_year = year - 1900;
    tmTime.tm_mon  = month - 1;
    tmTime.tm_mday = day;
    tmTime.tm_hour = hour;
    tmTime.tm_min  = minute;
    tmTime.tm_sec  = second;
    tmTime.tm_isdst = -1; // Let the system determine Daylight Saving Time

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

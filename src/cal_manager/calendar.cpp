#include "calendar.h"

/**
 * @brief Construct a new Calendar:: Calendar object
 * 
 * @param calId     Unique calendar Id
 * @param sum       Calendar summary (name)
 * @param desc      Calendar description
 * @param en        Is calendar enabled
 * @param prim      Is calendar primary
 */
Calendar::Calendar(std::string calId, std::string sum, std::string desc, bool en, bool prim)
{
    calendarId = calId;
    summary = sum;
    description = desc;
    enabled = en;
    primary = prim;
}

/**
 * @brief Construct a new Calendar:: Calendar object
 * 
 * @param calId     Unique calendar Id
 * @param sum       Calendar summary (name)
 * @param desc      Calendar description
 * @param prim      Is calendar primary
 */
Calendar::Calendar(std::string calId, std::string sum, std::string desc, bool prim) : Calendar(calId, sum, desc, true, prim)
{

}

/**
 * @brief Construct a new Calendar:: Calendar object
 * 
 * @param calId     Unique calendar Id
 * @param sum       Calendar summary (name)
 * @param desc      Calendar description
 */
Calendar::Calendar(std::string calId, std::string sum, std::string desc) : Calendar(calId, sum, desc, true, false)
{

}

Calendar::~Calendar()
{
}
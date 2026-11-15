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
Calendar::Calendar(const char *calId, const char *sum, const char *desc, bool en, bool prim) :
enabled(en),
primary(prim)
{
    strcpy(calendarId, calId);
    strcpy(summary, sum);
    strcpy(description, desc);
}

/**
 * @brief Construct a new Calendar:: Calendar object
 * 
 * @param calId     Unique calendar Id
 * @param sum       Calendar summary (name)
 * @param desc      Calendar description
 * @param prim      Is calendar primary
 */
Calendar::Calendar(const char *calId, const char *sum, const char *desc, bool prim) : Calendar(calId, sum, desc, true, prim)
{

}

/**
 * @brief Construct a new Calendar:: Calendar object
 * 
 * @param calId     Unique calendar Id
 * @param sum       Calendar summary (name)
 * @param desc      Calendar description
 */
Calendar::Calendar(const char *calId, const char *sum, const char *desc) : Calendar(calId, sum, desc, true, false)
{

}

Calendar::Calendar() : calendarId("\0"), summary("\0"), description("\0"), enabled(false), primary(false)
{

}

Calendar::~Calendar()
{
}
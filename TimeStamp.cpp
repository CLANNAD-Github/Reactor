
#include "TimeStamp.h"

std::string TimeStamp::string_now()
{
    return TimeStamp().to_string();
}

time_t TimeStamp::now()
{
    return time(NULL);
}

TimeStamp::TimeStamp() : m_time(time(NULL))
{}

TimeStamp::TimeStamp(time_t t) : m_time(t)
{}

void TimeStamp::update()
{
    m_time = time(NULL);
}

time_t TimeStamp::to_time() const
{
    return m_time;
}

std::string TimeStamp::to_string() const
{
    char buffer[20];
    memset(buffer, '\0', sizeof(buffer));

    struct tm tmp;
    localtime_r(&m_time, &tmp);    
    sprintf(buffer, "%4d-%02d-%02d %02d:%02d:%02d", \
        tmp.tm_year + 1900, tmp.tm_mon + 1, tmp.tm_mday,\
        tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
    return std::string(buffer);
}

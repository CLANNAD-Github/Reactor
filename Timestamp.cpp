
#include "Timestamp.h"

Timestamp::Timestamp()
{
    m_timeval = time(NULL);
}

Timestamp::Timestamp(const time_t t)
{
    m_timeval = t;
}

time_t Timestamp::toint() const
{
    return m_timeval;
}

std::string Timestamp::tostring() const
{
    char buf[30] = {0};
    struct tm* tm_time = localtime(&m_timeval);
    snprintf(buf, 30, "%4d-%02d-%02d %02d:%02d:%02d",\
        tm_time->tm_year + 1900,
        tm_time->tm_mon + 1,
        tm_time->tm_mday,
        tm_time->tm_hour,
        tm_time->tm_min,
        tm_time->tm_sec);
    return buf;
}

Timestamp Timestamp::now()
{
    return Timestamp();
}

Timestamp::~Timestamp()
{}


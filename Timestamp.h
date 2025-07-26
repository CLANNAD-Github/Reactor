#pragma once

#include <iostream>
#include <string>

class Timestamp
{
private:
    time_t m_timeval;
public:
    Timestamp();
    Timestamp(const time_t t);

    time_t toint() const;
    std::string tostring() const;

    static Timestamp now();

    ~Timestamp();
};

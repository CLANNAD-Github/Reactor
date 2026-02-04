
#include <stdio.h>
#include <time.h>
#include <string>
#include <string.h>

class TimeStamp
{
private:
    time_t m_time;
public:

    static std::string string_now();
    static time_t now();

    TimeStamp();
    TimeStamp(time_t m_time);
    ~TimeStamp() = default;
    
    std::string to_string() const;
    time_t to_time() const;
    void update();
};

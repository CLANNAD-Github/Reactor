#pragma once

#include <string>

enum class data_type
{
    T_NONE = 0,
    T_INT_HEAD = 1
};

class Buffer
{
private:
    std::string m_data;
    enum data_type m_type;
public:
    Buffer(enum data_type type);
    ~Buffer();

    bool empty() const;
    size_t size() const;

    bool pick_message(std::string& message);
    void append(const char* data, size_t size);
    int send_to_fd(int fd);
    int recv_from_fd(int fd);

    void erase(int pos, int size);
    void clear();
};

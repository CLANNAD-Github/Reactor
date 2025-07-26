#include <string>

class Buffer
{
private:
    std::string m_data;
    const int m_sep;  // 0：表示没有分隔符 1：表示 4 字节整数作为分隔符。 2：表示采用 "\r\n\r\n" http 协议分隔符
public:
    Buffer(const int sep);
    ~Buffer();

    void erase(int pos, int size);
    void append(const char* data, size_t size);
    void append_with_sep(const char* data, int size);
    size_t size() const;
    const char* data() const;
    void clear();
    bool pickmessage(std::string& message);
};


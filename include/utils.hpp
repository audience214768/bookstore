#include <exception>
#include <string>

class Exception : public std::exception {
private:
    std::string message;
public:
    explicit Exception(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
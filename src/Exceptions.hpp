#ifndef EXCEPTIONS_HPP_DEFINED
#define EXCEPTIONS_HPP_DEFINED

#include <stdexcept>
#include <string>

namespace utymap {

// Thrown when exception is occured due to mapcss stylsheet processing.
class MapCssException : public std::invalid_argument
{
public:
    MapCssException(std::string message) : invalid_argument(message.c_str()) {}
    ~MapCssException() throw() {}
};

}
#endif // EXCEPTIONS_HPP_DEFINED

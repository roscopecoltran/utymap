#ifndef UTILS_COREUTILS_HPP_DEFINED
#define UTILS_COREUTILS_HPP_DEFINED

#include <string>
#include <sstream>

namespace utymap { namespace utils {

template <typename T>
inline std::string toString(T t) {
    std::stringstream stream;
    stream << t;
    return stream.str();
}

}}

#endif // UTILS_COREUTILS_HPP_DEFINED

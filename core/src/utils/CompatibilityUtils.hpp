#ifndef UTILS_COMPATIBILITYUTILS_HPP_DEFINED
#define UTILS_COMPATIBILITYUTILS_HPP_DEFINED

#ifdef __CYGWIN__

#include <string>
#include <sstream>

namespace std
{
    template <typename T>
    inline std::string to_string(T t)
    {
        return static_cast<std::ostringstream*>( &(std::ostringstream() << t) )->str();
    }

    inline double stod(const std::string &Text)
    {
        std::istringstream ss(Text);
        double result;
        return ss >> result ? result : 0;
    }
}

#endif


#endif // UTILS_COMPATIBILITYUTILS_HPP_DEFINED

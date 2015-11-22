#include "QuadKey.hpp"

using namespace utymap;

std::string QuadKey::toString() const
{
    std::string code;
    code.reserve(levelOfDetail);
    for (int i = levelOfDetail; i > 0; --i)
    {
        char digit = '0';
        int mask = 1 << (i - 1);
        if ((tileX & mask) != 0)
            digit++;
        if ((tileY & mask) != 0)
        {
            digit++;
            digit++;
        }
        code += digit;
    }
    return code;
}

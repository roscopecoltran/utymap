#ifndef QUADKEY_HPP_DEFINED
#define QUADKEY_HPP_DEFINED

#include <string>

namespace utymap {

// Represents quadkey: a node of quadtree
struct QuadKey
{
    // Level of details (zoom).
    int levelOfDetail;
    // Tile x at given LOD
    int tileX;
    // Tile y at given LOD
    int tileY;

    // Constructs and returns string representation of quadkey.
    std::string toString() const
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
};

}
#endif // QUADKEY_HPP_DEFINED

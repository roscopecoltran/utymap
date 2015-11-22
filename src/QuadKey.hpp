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
    std::string toString() const;
};

}
#endif // QUADKEY_HPP_DEFINED

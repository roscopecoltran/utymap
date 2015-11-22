#ifndef INDEX_GEOUTILS_HPP_DEFINED
#define INDEX_GEOUTILS_HPP_DEFINED

#include "BoundingBox.hpp"

#include <cmath>
#include <string>

#ifdef M_PI
static double pi = M_PI;
#else
static double pi = std::acos(-1);
#endif

namespace utymap { namespace index {

class GeoUtils
{
public:

    // Converts Latitude/Longitude to quadkey
    static QuadKey latLonToQuadKey(double latitude, double longitude, int levelOfDetail)
    {
        QuadKey quadKey;
        quadKey.tileX = lonToTileX(longitude, levelOfDetail);
        quadKey.tileY = latToTileY(latitude, levelOfDetail);
        quadKey.levelOfDetail = levelOfDetail;
        return quadKey;
    }

    // Converts quadkey to bounding box
    static BoundingBox quadKeyToBoundingBox(const QuadKey& quadKey)
    {
        BoundingBox boundingBox;
        int levelOfDetail = quadKey.levelOfDetail;
        boundingBox.minLatitude = tileYToLat(quadKey.tileY + 1, levelOfDetail);
        boundingBox.maxLatitude = tileYToLat(quadKey.tileY, levelOfDetail);
        boundingBox.minLongitude = tileXToLon(quadKey.tileX, levelOfDetail);
        boundingBox.maxLongitude = tileXToLon(quadKey.tileX + 1, levelOfDetail);
        return boundingBox;
    }

private:

    inline static int lonToTileX(double lon, int levelOfDetail)
    {
        return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, levelOfDetail)));
    }

    inline static int latToTileY(double lat, int levelOfDetail)
    {
        return (int)(std::floor((1.0 - log(tan(lat * pi / 180.0) + 1.0 / cos(lat * pi / 180.0)) / pi) /
            2.0 * pow(2.0, levelOfDetail)));
    }

    inline static double tileXToLon(int x, int levelOfDetail)
    {
        return x / pow(2.0, levelOfDetail) * 360.0 - 180;
    }

    inline static double tileYToLat(int y, int levelOfDetail)
    {
        double n = pi - 2.0 * pi * y / pow(2.0, levelOfDetail);
        return 180.0 / pi * atan(0.5 * (exp(n) - exp(-n)));
    }
};

}}

#endif // INDEX_GEOUTILS_HPP_DEFINED

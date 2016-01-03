#ifndef INDEX_GEOUTILS_HPP_DEFINED
#define INDEX_GEOUTILS_HPP_DEFINED

#include "BoundingBox.hpp"
#include "QuadKey.hpp"

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
    static QuadKey latLonToQuadKey(const GeoCoordinate& coordinate, int levelOfDetail)
    {
        QuadKey quadKey;
        quadKey.tileX = lonToTileX(coordinate.longitude, levelOfDetail);
        quadKey.tileY = latToTileY(coordinate.latitude, levelOfDetail);
        quadKey.levelOfDetail = levelOfDetail;
        return quadKey;
    }

    // Converts quadkey to bounding box
    static BoundingBox quadKeyToBoundingBox(const QuadKey& quadKey)
    {
        GeoCoordinate minPoint, maxPoint;
        int levelOfDetail = quadKey.levelOfDetail;
        minPoint.latitude = tileYToLat(quadKey.tileY + 1, levelOfDetail);
        maxPoint.latitude = tileYToLat(quadKey.tileY, levelOfDetail);
        minPoint.longitude = tileXToLon(quadKey.tileX, levelOfDetail);
        maxPoint.longitude = tileXToLon(quadKey.tileX + 1, levelOfDetail);
        
        return BoundingBox { minPoint, maxPoint };
    }

    // Visits all tiles which are intersecting with given bounding box at given level of details
    template<typename Visitor>
    static void visitTileRange(BoundingBox bbox, int levelOfDetail, Visitor& visitor)
    {
        QuadKey start = latLonToQuadKey(bbox.minPoint, levelOfDetail);
        QuadKey end = latLonToQuadKey(bbox.maxPoint, levelOfDetail);

        for (int y = end.tileY; y < start.tileY + 1; y++) {
            for (int x = start.tileX; x < end.tileX + 1; x++) {
                const QuadKey currentQuadKey = { levelOfDetail, x, y };
                const BoundingBox currentBbox = quadKeyToBoundingBox(currentQuadKey);
                if (bbox.intersects(currentBbox)) {
                    visitor(currentQuadKey, currentBbox);
                }
            }
        }
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

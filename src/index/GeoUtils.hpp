#ifndef INDEX_GEOUTILS_HPP_DEFINED
#define INDEX_GEOUTILS_HPP_DEFINED

#include "BoundingBox.hpp"
#include "QuadKey.hpp"

#include <algorithm>
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
    static const int MinLevelOfDetails = 1;
    static const int MaxLevelOfDetails = 16;

    // Converts Latitude/Longitude to quadkey
    static QuadKey latLonToQuadKey(const GeoCoordinate& coordinate, int levelOfDetail)
    {
        QuadKey quadKey;
        quadKey.tileX = lonToTileX(clip(coordinate.longitude, -179.9999999, 179.9999999), levelOfDetail);
        quadKey.tileY = latToTileY(clip(coordinate.latitude, -85.05112877, 85.05112877), levelOfDetail);
        quadKey.levelOfDetail = levelOfDetail;
        return quadKey;
    }

    // clips value in range
    inline static double clip(double n, double minValue, double maxValue)
    {
        return std::max(minValue, std::min(n, maxValue));
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
        if (!bbox.isValid()) 
            return;

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

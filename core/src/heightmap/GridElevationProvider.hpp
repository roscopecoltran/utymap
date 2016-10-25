#ifndef HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED
#define HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"
#include "utils/GeoUtils.hpp"

#include <string>
#include <stdexcept>
#include <sstream>

namespace utymap { namespace heightmap {

// Provides the way to get elevation for given location from grid 
// represented by comma separated list of integers
class GridElevationProvider final : public ElevationProvider
{
public:
    GridElevationProvider(std::string dataDirectory) :
        dataDirectory_(dataDirectory)
    {
    }

    void preload(const utymap::BoundingBox& boundingBox) override
    {
        throw std::domain_error("Not implemented.");
    }

    /// Preloads data for given quadkey.
    void preload(const utymap::QuadKey& quadkey) override 
    {

    }

    /// Gets elevation for given geocoordinate.
    double getElevation(const utymap::QuadKey& quadKey, const utymap::GeoCoordinate& coordinate) const override
    {
        return 0;
    }

    /// Gets elevation for given geocoordinate.
    double getElevation(const utymap::QuadKey& quadKey, double latitude, double longitude) const override
    {
        return 0;
    }

private:

    std::string getFilePath(const QuadKey& quadKey) const
    {
        std::stringstream ss;
        ss << dataDirectory_ << quadKey.levelOfDetail << "/" << utymap::utils::GeoUtils::quadKeyToString(quadKey) << ".ele";
        return ss.str();
    }

    const std::string dataDirectory_;
};

}}

#endif // HEIGHTMAP_GRIDELEVATIONPROVIDER_HPP_DEFINED

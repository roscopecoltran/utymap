#ifndef UTILS_MESHUTILS_HPP_DEFINED
#define UTILS_MESHUTILS_HPP_DEFINED

#include "math/Mesh.hpp"
#include "math/Vector3.hpp"

namespace utymap { namespace utils {

/// Copies mesh into existing one adjusting position.
inline void copyMesh(const utymap::math::Vector3& position, const utymap::math::Mesh& source, utymap::math::Mesh& destination)
{
    int startIndex = static_cast<int>(destination.vertices.size() / 3);

    // copy adjusted vertices
    for (std::size_t i = 0; i < source.vertices.size();) {
        destination.vertices.push_back(source.vertices[i++] + position.x);
        destination.vertices.push_back(source.vertices[i++] + position.z);
        destination.vertices.push_back(source.vertices[i++] + position.y);
    }

    // copy adjusted triangles
    std::transform(source.triangles.begin(), source.triangles.end(), std::back_inserter(destination.triangles), [&](int value) {
        return value + startIndex;
    });

    // copy colors
    std::copy(source.colors.begin(), source.colors.end(), std::back_inserter(destination.colors));

    // copy adjusted uvs
    std::copy(source.uvs.begin(), source.uvs.end(), std::back_inserter(destination.uvs));
    std::copy(source.uvMap.begin(), source.uvMap.end(), std::back_inserter(destination.uvMap));

    startIndex = static_cast<int>(destination.uvs.size() - source.uvs.size());
    for (std::size_t i = destination.uvMap.size() - source.uvMap.size(); i < destination.uvMap.size(); i += 8) {
        destination.uvMap[i] += startIndex;
    }
}

/// Copies mesh along two coordinates.
inline void copyMeshAlong(const utymap::QuadKey& quadKey, const utymap::GeoCoordinate& p1, const utymap::GeoCoordinate& p2,
                          const utymap::math::Mesh& source, utymap::math::Mesh& destination, double stepInMeters,
                          const utymap::heightmap::ElevationProvider& eleProvider)
{
    double distanceInMeters = GeoUtils::distance(p1, p2);
    int count = static_cast<int>(distanceInMeters / stepInMeters);

    for (int j = 0; j < count; ++j) {
        GeoCoordinate position = GeoUtils::newPoint(p1, p2, static_cast<double>(j) / count);

        double elevation = eleProvider.getElevation(quadKey, position);
        utymap::utils::copyMesh(utymap::math::Vector3(position.longitude, elevation, position.latitude), source, destination);
    }
}

}}

#endif // UTILS_GEOUTILS_HPP_DEFINED
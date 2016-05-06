#ifndef MESHING_MESHBUILDER_HPP_DEFINED
#define MESHING_MESHBUILDER_HPP_DEFINED

#include "heightmap/ElevationProvider.hpp"
#include "mapcss/Color.hpp"
#include "mapcss/ColorGradient.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"

#include <limits>
#include <memory>

namespace utymap { namespace meshing {

// Provides the way to build mesh in 3D space.
class MeshBuilder
{
public:
    struct Options
    {
        // Max area of triangle in refined mesh.
        double area;
        // Elevation noise freq.
        double eleNoiseFreq;
        // Color noise freq.
        double colorNoiseFreq;
        // Height offset.
        double heightOffset;
        // Fixed elevation.
        double elevation;
        // Gradient data
        const utymap::mapcss::ColorGradient& gradient;

        // Flag indicating whether to suppress boundary segment splitting.
        //     0 = split segments (default)
        //     1 = no new vertices on the boundary
        //     2 = prevent all segment splitting, including internal boundaries
        int segmentSplit;

        Options(double area,
                double eleNoiseFreq,
                double colorNoiseFreq,
                double heightOffset,
                const utymap::mapcss::ColorGradient& gradient,
                double elevation = std::numeric_limits<double>::lowest(),
                int segmentSplit = 0) :
            area(area), 
            eleNoiseFreq(eleNoiseFreq), 
            colorNoiseFreq(colorNoiseFreq),
            heightOffset(heightOffset),
            gradient(gradient),
            elevation(elevation),
            segmentSplit(segmentSplit)
        {
        }
    };

    // Creates builder with given elevation provider.
    MeshBuilder(const utymap::heightmap::ElevationProvider& eleProvider);
    ~MeshBuilder();

    // Adds polygon to existing mesh using options provided.
    void addPolygon(Mesh& mesh, Polygon& polygon, const MeshBuilder::Options& options) const;

    // Adds simple plane to existing mesh using options provided.
    void addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, const MeshBuilder::Options& options) const;

    // Adds simple plane to existing mesh using elevation and options provided.
    void addPlane(Mesh& mesh, const Vector2& p1, const Vector2& p2, double ele1, double ele2, const MeshBuilder::Options& options) const;

    // Adds triangle to mesh.
    void addTriangle(Mesh& mesh,
                     const utymap::meshing::Vector3& v0,
                     const utymap::meshing::Vector3& v1,
                     const utymap::meshing::Vector3& v2,
                     const MeshBuilder::Options& options,
                     bool hasBackSide) const;

private:
    class MeshBuilderImpl;
    std::unique_ptr<MeshBuilderImpl> pimpl_;
};

}}

#endif // MESHING_MESHBUILDER_HPP_DEFINED

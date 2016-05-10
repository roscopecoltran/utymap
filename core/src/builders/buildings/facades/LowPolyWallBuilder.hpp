#ifndef BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/BuilderContext.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"
#include "mapcss/ColorGradient.hpp"
#include "utils/GradientUtils.hpp"

namespace utymap { namespace builders {

// Responsible for building facade wall in low poly quality.
class LowPolyWallBuilder
{
public:
    LowPolyWallBuilder(utymap::meshing::Mesh& mesh,
                       const utymap::mapcss::ColorGradient& gradient,
                       const utymap::builders::BuilderContext& context) :
        mesh_(mesh), context_(context), options_(0, 0, 0, 12, gradient), minHeight_(0)
    {
    }

    // Sets height of wall.
    inline LowPolyWallBuilder& setHeight(double height) { options_.heightOffset = height; return *this; }

    // Sets height above ground level.
    inline LowPolyWallBuilder& setMinHeight(double minHeight) { minHeight_ = minHeight; return *this; }

    // Sets color freq.
    inline LowPolyWallBuilder& setColorNoise(double freq) { options_.colorNoiseFreq = freq; return *this; }

    // Adds vertical plane which has right side orientation.
    void build(const utymap::GeoCoordinate& start, const utymap::GeoCoordinate& end)
    {
        addPlane(start, end);
    }

private:

    inline void addPlane(const utymap::GeoCoordinate& p1, const utymap::GeoCoordinate& p2)
    {
        context_.meshBuilder.addPlane(mesh_,
            utymap::meshing::Vector2(p1.longitude, p1.latitude),
            utymap::meshing::Vector2(p2.longitude, p2.latitude),
            minHeight_,
            minHeight_,
            options_);
    }

    utymap::meshing::Mesh& mesh_;
    const utymap::builders::BuilderContext& context_;
    utymap::meshing::MeshBuilder::Options options_;
    double minHeight_;
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

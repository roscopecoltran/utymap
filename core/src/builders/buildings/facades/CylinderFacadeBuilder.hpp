#ifndef BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED

#include "builders/buildings/facades/FacadeBuilder.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/GeometryUtils.hpp"

namespace utymap { namespace builders {

// Responsible for building cylinder shaped facade.
class CylinderFacadeBuilder : public FacadeBuilder
{
public:
    CylinderFacadeBuilder(const utymap::builders::BuilderContext& builderContext,
                          utymap::builders::MeshContext& meshContext)
            : FacadeBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon)
    {
        utymap::utils::outerRectangles(polygon, [&](const utymap::meshing::Rectangle& rectangle) {
            utymap::meshing::Vector2 center2d;
            double radius;
            utymap::utils::getCircle(rectangle, center2d, radius);

            CylinderGenerator cylinderGenerator(builderContext_, meshContext_);
            cylinderGenerator
                .setCenter(utymap::meshing::Vector3(center2d.x, minHeight_, center2d.y))
                .setHeight(height_)
                .setMaxSegmentHeight(5)
                .setRadialSegments(7)
                .setRadius(radius)
                .setColorNoiseFreq(0)
                .generate();
        });
    }
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_CYLINDERFACADEBUILDER_HPP_DEFINED

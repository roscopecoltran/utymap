#ifndef BUILDERS_BUILDINGS_FACADES_FLATFACADEBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_FLATFACADEBUILDER_HPP_DEFINED

#include "builders/buildings/facades/FacadeBuilder.hpp"
#include "meshing/MeshBuilder.hpp"

namespace utymap { namespace builders {

// Responsible for building facade wall in low poly quality.
class FlatFacadeBuilder : public FacadeBuilder
{
public:
    FlatFacadeBuilder(const utymap::builders::BuilderContext& builderContext,
                      utymap::builders::MeshContext& meshContext)
        : FacadeBuilder(builderContext, meshContext)
    {
    }

  void build(utymap::meshing::Polygon& polygon)
  {
    utymap::meshing::MeshBuilder::Options options(
        0, // area
        0, // ele noise
        colorNoiseFreq_,
        height_,
        getColorGradient(),
        minHeight_
    );

    for (const auto& outer : polygon.outers)
        buildRange(polygon, outer, options);

    for (const auto& inner : polygon.inners)
        buildRange(polygon, inner, options);
  }

private:

    inline void buildRange(const utymap::meshing::Polygon& polygon,
                           const utymap::meshing::Polygon::Range& range, 
                           const utymap::meshing::MeshBuilder::Options& options)
    {
        std::int64_t first = range.first;
        std::int64_t last = range.second - 2;
        for (std::int64_t i = last; i >= first; i -= 2) {
            utymap::meshing::Vector2 p1(polygon.points[i], polygon.points[i + 1]);
            auto j = i == first ? last : i - 2;
            utymap::meshing::Vector2 p2(polygon.points[j], polygon.points[j + 1]);

            builderContext_.meshBuilder.addPlane(meshContext_.mesh,
                p1,
                p2,
                minHeight_,
                minHeight_,
                options);
        }
    }
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_FLATFACADEBUILDER_HPP_DEFINED

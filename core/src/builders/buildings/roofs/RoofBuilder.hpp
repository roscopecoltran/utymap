#ifndef BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"

namespace utymap { namespace builders {

/// Specifies base roof builder functionality.
class RoofBuilder
{
public:
    RoofBuilder(const utymap::builders::BuilderContext& builderContext,
                utymap::builders::MeshContext& meshContext) :
        builderContext_(builderContext), 
        meshContext_(meshContext),
        height_(0), 
        minHeight_(0)
    {
    }

    virtual ~RoofBuilder() = default;

    // Sets height.
    RoofBuilder& setHeight(double height)
    {
        height_ = height; 
        return *this;
    }

    // Sets height above ground level.
    RoofBuilder& setMinHeight(double minHeight)
    {
        minHeight_ = minHeight; 
        return *this;
    }

    // Sets color noise freq.
    RoofBuilder& setColorNoiseFreq(double noiseFreq)
    {
        meshContext_.appearanceOptions.colorNoiseFreq = noiseFreq;
        return *this;
    }

    // Builds roof from polygon.
    virtual void build(utymap::meshing::Polygon& polygon) = 0;

protected:
    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext& meshContext_;
    double height_, minHeight_;
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_ROOFBUILDER_HPP_DEFINED

#ifndef BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED

#include "builders/terrain/TerraExtras.hpp"
#include "builders/terrain/TerraGenerator.hpp"
#include "math/Mesh.hpp"
#include "math/Polygon.hpp"
#include "math/Vector2.hpp"

namespace utymap { namespace builders {

/// Provides the way to generate terrain mesh.
class SurfaceGenerator final : public TerraGenerator
{
public:
    SurfaceGenerator(const BuilderContext& context,
                     const utymap::mapcss::Style& style,
                     const ClipperLib::Path& tileRect);

    void onNewRegion(const std::string& type,
                     const utymap::entities::Element& element,
                     const utymap::mapcss::Style& style,
                     const std::shared_ptr<Region>& region) override;

    void generateFrom(Layers& layers) override;

private:
    /// Builds foreground surface.
    void buildForeground(Layers& layers);

    /// Builds background surface.
    void buildBackground();

    /// Builds layer.
    void buildLayer(Layer& layer);

    /// Builds mesh using paths data.
    void buildFromPaths(const ClipperLib::Paths& paths, const RegionContext& regionContext);

    /// Builds height contour shape.
    void buildHeightOffset(const std::vector<utymap::math::Vector2>& points, const RegionContext& regionContext);

    /// Adds geometry to mesh.
    void addGeometry(ClipperLib::Paths& paths, const RegionContext& regionContext);

    /// Adds geometry to mesh.
    void addGeometry(utymap::math::Polygon& polygon, const RegionContext& regionContext);

    /// Adds extras to mesh, e.g. trees, water surface if meshExtras are specified in options.
    void addExtrasIfNecessary(utymap::math::Mesh& mesh,
                              TerraExtras::Context& extrasContext,
                              const RegionContext& regionContext) const;

    ClipperLib::ClipperEx foregroundClipper_;
    ClipperLib::ClipperEx backgroundClipper_;
};

}}
#endif // BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED

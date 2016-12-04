#ifndef BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "builders/terrain/TerraExtras.hpp"
#include "builders/terrain/TerraGenerator.hpp"
#include "builders/terrain/RegionTypes.hpp"
#include "math/Mesh.hpp"
#include "math/Polygon.hpp"
#include "math/Rectangle.hpp"
#include "math/Vector2.hpp"

namespace utymap { namespace builders {

/// Provides the way to generate terrain mesh.
class SurfaceGenerator final : public TerraGenerator
{
public:
    SurfaceGenerator(const BuilderContext& context,
                     const utymap::mapcss::Style& style,
                     const ClipperLib::Path& tileRect);

    /// Adds region
    void addRegion(const std::string& type,
                   const utymap::entities::Element& element,
                   const utymap::mapcss::Style& style,
                   std::shared_ptr<Region> region) override;

    /// Generates mesh and calls callback from context.
    void generate() override;

private:
    /// Builds all objects for quadkey organized by layers
    void buildLayers();

    /// Builds background as clip area of layers
    void buildBackground();

    void buildFromRegions(Regions& regions, const RegionContext& regionContext);

    void buildFromPaths(const ClipperLib::Paths& paths, const RegionContext& regionContext);

    void populateMesh(ClipperLib::Paths& paths, const RegionContext& regionContext);

    std::vector<utymap::math::Vector2> restorePoints(const ClipperLib::Path& path) const;

    void fillMesh(utymap::math::Polygon& polygon, const RegionContext& regionContext);

    /// Adds extras to mesh, e.g. trees, water surface if meshExtras are specified in options.
    void addExtrasIfNecessary(utymap::math::Mesh& mesh,
                              TerraExtras::Context& extrasContext,
                              const RegionContext& regionContext) const;

    void processHeightOffset(const std::vector<utymap::math::Vector2>& points, 
                             const RegionContext& regionContext);

    ClipperLib::ClipperEx foregroundClipper_;
    ClipperLib::ClipperEx backgroundClipper_;
    LineGridSplitter splitter_;
    utymap::math::Mesh mesh_;
    Layers layers_;
    utymap::math::Rectangle rect_;
};

}}
#endif // BUILDERS_TERRAIN_SURFACEGENERATOR_HPP_DEFINED

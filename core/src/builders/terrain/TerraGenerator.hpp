#ifndef BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "builders/terrain/TerraExtras.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <memory>
#include <unordered_map>
#include <queue>
#include <vector>

namespace utymap { namespace builders {

/// Provides the way to generate terrain mesh.
class TerraGenerator final
{
public:
    /// Region context encapsulates information about given region.
    struct RegionContext final
    {
        const utymap::mapcss::Style style;
        // Prefix in mapcss.
        const std::string prefix;

        const utymap::meshing::MeshBuilder::GeometryOptions geometryOptions;
        const utymap::meshing::MeshBuilder::AppearanceOptions appearanceOptions;

        RegionContext(const utymap::mapcss::Style& style,
                      const std::string& prefix,
                      const utymap::meshing::MeshBuilder::GeometryOptions& geometryOptions,
                      const utymap::meshing::MeshBuilder::AppearanceOptions& appearanceOptions) :
            style(style), 
            prefix(prefix), 
            geometryOptions(std::move(geometryOptions)), 
            appearanceOptions(std::move(appearanceOptions))
        {
        }
    };

    /// Represents terrain region.
    struct Region final
    {
        Region() : 
            isLayer(false), area(0), context(nullptr), points() 
        {
        }

        Region(Region&& other) :
            isLayer(other.isLayer), area(other.area), 
            context(std::move(other.context)), points(std::move(other.points))
        {
        };

        Region(const Region&) = delete;
        Region&operator=(const Region&) = delete;
        Region&operator=(Region&&) = delete;

        bool isLayer;
        double area;
        std::unique_ptr<RegionContext> context; // optional: might be empty if polygon is layer
        ClipperLib::Paths points;
    };

    TerraGenerator(const BuilderContext& context,
                   const utymap::mapcss::Style& style,
                   ClipperLib::ClipperEx& foregroundClipper_);

    /// Adds region
    void addRegion(const std::string& type, std::unique_ptr<Region> region);

    /// Generates mesh and calls callback from context.
    void generate(ClipperLib::Path& tileRect);

    /// Creates region  context.
    RegionContext createRegionContext(const utymap::mapcss::Style& style,
                                      const std::string& prefix) const;

private:
    typedef std::unique_ptr<Region> RegionPtr;
    typedef std::vector<utymap::meshing::Vector2> Points;


    struct GreaterThanByArea
    {
        bool operator()(const RegionPtr& lhs, const RegionPtr& rhs) const
        {
            return lhs->area > rhs->area;
        }
    };

    typedef std::priority_queue<RegionPtr, std::vector<RegionPtr>, GreaterThanByArea> Regions;
    typedef std::unordered_map<std::string, Regions> Layers;

    /// Builds all objects for quadkey organized by layers
    void buildLayers();

    /// Builds background as clip area of layers
    void buildBackground(ClipperLib::Path& tileRect);

    void buildFromRegions(Regions& regions, const RegionContext& regionContext);

    void buildFromPaths(const ClipperLib::Paths& paths, const RegionContext& regionContext);

    void populateMesh(ClipperLib::Paths& paths, const RegionContext& regionContext);

    Points restorePoints(const ClipperLib::Path& path) const;

    void fillMesh(utymap::meshing::Polygon& polygon, const RegionContext& regionContext);

    /// Adds extras to mesh, e.g. trees, water surface if meshExtras are specified in options.
    void addExtrasIfNecessary(utymap::meshing::Mesh& mesh,
                              TerraExtras::Context& extrasContext,
                              const RegionContext& regionContext) const;

    void processHeightOffset(const Points& points, const RegionContext& regionContext);

    const BuilderContext& context_;
    const utymap::mapcss::Style& style_;
    ClipperLib::ClipperEx& foregroundClipper_;
    ClipperLib::ClipperEx backGroundClipper_;
    LineGridSplitter splitter_;
    utymap::meshing::Mesh mesh_;
    Layers layers_;
    utymap::meshing::Rectangle rect_;
};

}}

#endif // BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

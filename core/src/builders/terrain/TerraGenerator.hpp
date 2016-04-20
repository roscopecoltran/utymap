#ifndef BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

#include "BoundingBox.hpp"
#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/LineGridSplitter.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

#include <memory>
#include <unordered_map>
#include <vector>

namespace utymap { namespace builders {

// Provides the way to generate terrain mesh.
class TerraGenerator
{
public:

    // Represents terrain region points.
    struct Region
    {
        bool isLayer;
        std::shared_ptr<utymap::meshing::MeshBuilder::Options> options; // optional: might be empty if polygon is layer
        ClipperLib::Paths points;
    };

    TerraGenerator(const BuilderContext& context,
                   const utymap::BoundingBox& bbox,
                   const utymap::mapcss::Style& style,
                   ClipperLib::ClipperEx& clipper);

    void addRegion(const std::string& type, const Region& region);

    void generate(ClipperLib::Path& tileRect);

    std::shared_ptr<utymap::meshing::MeshBuilder::Options> createMeshOptions(const utymap::mapcss::Style& style,
                                                                             const std::string& prefix);

private:
    typedef std::vector<utymap::meshing::Point> Points;
    typedef std::vector<Region> Regions;
    typedef std::unordered_map<std::string, Regions> Layers;

    void buildLayers();

    void buildBackground(ClipperLib::Path& tileRect);

    void buildFromRegions(const Regions& regions,
                          const std::shared_ptr<utymap::meshing::MeshBuilder::Options>& options);

    void buildFromPaths(ClipperLib::Paths& paths,
                        const std::shared_ptr<utymap::meshing::MeshBuilder::Options>& options);

    void populateMesh(ClipperLib::Paths& paths,
                      const std::shared_ptr<utymap::meshing::MeshBuilder::Options>& options);

    Points restorePoints(const ClipperLib::Path& path);

    void fillMesh(utymap::meshing::Polygon& polygon,
                  const std::shared_ptr<utymap::meshing::MeshBuilder::Options>& options);

    void processHeightOffset(const Points& points,
                             const std::shared_ptr<utymap::meshing::MeshBuilder::Options>& options);

    const BuilderContext& context_;
    const utymap::mapcss::Style& style_;
    ClipperLib::ClipperEx& clipper_;
    LineGridSplitter splitter_;
    utymap::meshing::Mesh mesh_;
    Layers layers_;
    const utymap::BoundingBox& bbox_;
    utymap::meshing::Rectangle rect_;
};

}}

#endif // BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

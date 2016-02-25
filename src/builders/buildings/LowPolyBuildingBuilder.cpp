#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "meshing/MeshTypes.hpp"
#include "mapcss/ColorGradient.hpp"
#include "builders/buildings/facades/LowPolyWallBuilder.hpp"
#include "builders/buildings/roofs/LowPolyRoofBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "utils/GradientUtils.hpp"
#include "utils/MapCssUtils.hpp"

#include <vector>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;

namespace utymap { namespace builders {

class LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl : public utymap::entities::ElementVisitor
{
public:
    LowPolyBuildingBuilderImpl(const utymap::QuadKey& quadKey,
                               const utymap::mapcss::StyleProvider& styleProvider,
                               utymap::index::StringTable& stringTable,
                               utymap::heightmap::ElevationProvider& eleProvider,
                               std::function<void(const utymap::meshing::Mesh&)> callback):
        quadKey_(quadKey), styleProvider_(styleProvider), stringTable_(stringTable),
        eleProvider_(eleProvider), callback_(callback)
    {
    }

    void visitNode(const utymap::entities::Node&) { }

    void visitWay(const utymap::entities::Way&) { }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = styleProvider_.forElement(area, quadKey_.levelOfDetail);
        std::string gradientKey = utymap::utils::getString("color", stringTable_, style);
        ColorGradient gradient = styleProvider_.getGradient(gradientKey);

        Mesh mesh;
        mesh.name = "building";
        MeshBuilder meshBuilder(eleProvider_);
        LowPolyFlatRoofBuilder roofBuilder(mesh, gradient, meshBuilder);
        Polygon polygon(area.coordinates.size(), 0);
        polygon.addContour(toPoints(area.coordinates));
        roofBuilder.build(polygon);
        // TODO add floor

        LowPolyWallBuilder wallBuilder(mesh, gradient);
        int last = area.coordinates.size() - 1;
        for (auto i = 0; i <= last; ++i) {
            wallBuilder.build(area.coordinates[i], area.coordinates[i != last ? i + 1 : 0]);
        }
        callback_(mesh);
    }

    void visitRelation(const utymap::entities::Relation&)
    {
        // TODO
    }

private:

    inline std::vector<Point> toPoints(const std::vector<GeoCoordinate>& coordinates) const
    {
        std::vector<Point> points;
        points.reserve(coordinates.size());
        for (const auto& coordinate : coordinates) {
            points.push_back(Point(coordinate.longitude, coordinate.latitude));
        }

        return std::move(points);
    }

const QuadKey& quadKey_;
const StyleProvider& styleProvider_;
ElevationProvider& eleProvider_;
StringTable& stringTable_;
std::function<void(const Mesh&)> callback_;

};

LowPolyBuildingBuilder::LowPolyBuildingBuilder(const QuadKey& quadKey,
                                               const StyleProvider& styleProvider,
                                               StringTable& stringTable,
                                               ElevationProvider& eleProvider,
                                               std::function<void(const Mesh&)> callback) :
    pimpl_(new LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl(quadKey, styleProvider, stringTable, eleProvider, callback))
{
}

LowPolyBuildingBuilder::~LowPolyBuildingBuilder() { }

void LowPolyBuildingBuilder::visitNode(const utymap::entities::Node&) { }

void LowPolyBuildingBuilder::visitWay(const utymap::entities::Way&) { }

void LowPolyBuildingBuilder::visitArea(const utymap::entities::Area& area)
{
    area.accept(*pimpl_);
}

void LowPolyBuildingBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    relation.accept(*pimpl_);
}

}}

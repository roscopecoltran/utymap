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

class LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl : public ElementBuilder
{
public:
    LowPolyBuildingBuilderImpl(const utymap::builders::BuilderContext& context) :
        ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node&) { }

    void visitWay(const utymap::entities::Way&) { }

    void visitArea(const utymap::entities::Area& area)
    {
        Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);
        std::string gradientKey = utymap::utils::getString("color", context_.stringTable, style);
        ColorGradient gradient = context_.styleProvider.getGradient(gradientKey);

        Mesh mesh("building");
        LowPolyFlatRoofBuilder roofBuilder(mesh, gradient, context_.meshBuilder);
        Polygon polygon(area.coordinates.size(), 0);
        polygon.addContour(toPoints(area.coordinates));
        roofBuilder.build(polygon);
        // TODO add floor

        LowPolyWallBuilder wallBuilder(mesh, gradient, context_.meshBuilder);
        int last = area.coordinates.size() - 1;
        for (auto i = 0; i <= last; ++i) {
            wallBuilder.build(area.coordinates[i], area.coordinates[i != last ? i + 1 : 0]);
        }
        context_.meshCallback(mesh);
    }

    void visitRelation(const utymap::entities::Relation&)
    {
        // TODO
    }

    void complete()
    {
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
};

LowPolyBuildingBuilder::LowPolyBuildingBuilder(const utymap::builders::BuilderContext& context) :
    utymap::builders::ElementBuilder(context),
    pimpl_(new LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl(context))
{
}

LowPolyBuildingBuilder::~LowPolyBuildingBuilder() { }

void LowPolyBuildingBuilder::visitNode(const utymap::entities::Node&) { }

void LowPolyBuildingBuilder::visitWay(const utymap::entities::Way&) { }

void LowPolyBuildingBuilder::visitArea(const utymap::entities::Area& area)
{
    area.accept(*pimpl_);
}

void LowPolyBuildingBuilder::complete()
{
    pimpl_->complete();
}

void LowPolyBuildingBuilder::visitRelation(const utymap::entities::Relation& relation)
{
    relation.accept(*pimpl_);
}

}}

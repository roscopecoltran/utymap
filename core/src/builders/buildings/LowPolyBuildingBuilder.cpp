#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "meshing/MeshTypes.hpp"
#include "mapcss/ColorGradient.hpp"
#include "builders/buildings/facades/LowPolyWallBuilder.hpp"
#include "builders/buildings/roofs/LowPolyRoofBuilder.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "utils/MapCssUtils.hpp"

#include <unordered_map>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;

namespace {

    const std::string RoofTypeKey = "roof-type";
    const std::string RoofHeightKey = "roof-height";

    typedef std::function<std::shared_ptr<RoofBuilder>(Mesh&, const ColorGradient&, const BuilderContext&)> RoofBuilderFactory;
    static std::unordered_map<std::string, RoofBuilderFactory> RoofBuilderFactoryMap = 
    {
        { 
            "flat", 
            [](Mesh& m, const ColorGradient& c, const BuilderContext& ctx) { 
                return std::shared_ptr<LowPolyFlatRoofBuilder>(new LowPolyFlatRoofBuilder(m, c, ctx));
            } 
        },
        {
            "dome",
            [](Mesh& m, const ColorGradient& c, const BuilderContext& ctx) {
                return std::shared_ptr<DomeRoofBuilder>(new DomeRoofBuilder(m, c, ctx));
            }
        }
    };
}

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
        auto gradientKey = utymap::utils::getString("color", context_.stringTable, style);
        ColorGradient gradient = context_.styleProvider.getGradient(*gradientKey);
        double height = utymap::utils::getDouble("height", context_.stringTable, style);
        double minHeight = context_.eleProvider.getElevation(area.coordinates[0]);
        
        // roof
        auto roofType = utymap::utils::getString(RoofTypeKey, context_.stringTable, style, "flat");
        double roofHeight = utymap::utils::getDouble(RoofHeightKey, context_.stringTable, style, 0);

        Mesh mesh("building");

        Polygon polygon(area.coordinates.size(), 0);
        polygon.addContour(toPoints(area.coordinates));

        auto roofBuilder = RoofBuilderFactoryMap.find(*roofType)->second(mesh, gradient, context_);
        roofBuilder->setHeight(roofHeight);
        roofBuilder->setMinHeight(minHeight + height);
        roofBuilder->build(polygon);
        
        // TODO add floor

        LowPolyWallBuilder wallBuilder(mesh, gradient, context_);
        wallBuilder
            .setHeight(height)
            .setMinHeight(minHeight);
        int last = area.coordinates.size() - 1;
        for (auto i = last; i >= 0; --i) 
            wallBuilder.build(area.coordinates[i], area.coordinates[i != 0 ? i - 1 : last]);

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

    inline std::vector<Vector2> toPoints(const std::vector<GeoCoordinate>& coordinates) const
    {
        std::vector<Vector2> points;
        points.reserve(coordinates.size());
        for (const auto& coordinate : coordinates) {
            points.push_back(Vector2(coordinate.longitude, coordinate.latitude));
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

#include "GeoCoordinate.hpp"
#include "builders/MeshContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "builders/buildings/facades/FlatFacadeBuilder.hpp"
#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"

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
    const std::string FacadeTypeKey = "facade-type";

    typedef std::function<std::shared_ptr<RoofBuilder>(const BuilderContext&, MeshContext&)> RoofBuilderFactory;
    std::unordered_map<std::string, RoofBuilderFactory> RoofBuilderFactoryMap =
    {
        { 
            "flat", 
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::shared_ptr<LowPolyFlatRoofBuilder>(new LowPolyFlatRoofBuilder(builderContext, meshContext));
            } 
        },
        {
            "dome",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::shared_ptr<DomeRoofBuilder>(new DomeRoofBuilder(builderContext, meshContext));
            }
        }
    };

    typedef std::function<std::shared_ptr<FacadeBuilder>(const BuilderContext&, MeshContext&)> FacadeBuilderFactory;
    std::unordered_map<std::string, FacadeBuilderFactory> FacadeBuilderFactoryMap =
    {
        {
            "flat",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
              return std::shared_ptr<FlatFacadeBuilder>(new FlatFacadeBuilder(builderContext, meshContext));
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
        double height = utymap::utils::getDouble("height", context_.stringTable, style);
        double minHeight = context_.eleProvider.getElevation(area.coordinates[0]);

        // facade
        auto facadeType = utymap::utils::getString(FacadeTypeKey, context_.stringTable, style, "flat");
        
        // roof
        auto roofType = utymap::utils::getString(RoofTypeKey, context_.stringTable, style, "flat");
        double roofHeight = utymap::utils::getDouble(RoofHeightKey, context_.stringTable, style, 0);

        Mesh mesh("building");
        MeshContext meshContext(mesh, style);

        Polygon polygon(area.coordinates.size(), 0);
        polygon.addContour(toPoints(area.coordinates));

        auto roofBuilder = RoofBuilderFactoryMap.find(*roofType)->second(context_, meshContext);
        roofBuilder->setHeight(roofHeight);
        roofBuilder->setMinHeight(minHeight + height);
        roofBuilder->build(polygon);
        
        auto facadeBuilder = FacadeBuilderFactoryMap.find(*facadeType)->second(context_, meshContext);
        facadeBuilder->setHeight(height);
        facadeBuilder->setMinHeight(minHeight);
        facadeBuilder->build(polygon);

        // TODO add floor

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

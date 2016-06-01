#include "GeoCoordinate.hpp"
#include "builders/MeshContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "builders/buildings/facades/CylinderFacadeBuilder.hpp"
#include "builders/buildings/facades/FlatFacadeBuilder.hpp"
#include "builders/buildings/facades/SphereFacadeBuilder.hpp"
#include "builders/buildings/roofs/DomeRoofBuilder.hpp"
#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "builders/buildings/roofs/PyramidalRoofBuilder.hpp"
#include "builders/buildings/roofs/MansardRoofBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "utils/ElementUtils.hpp"

#include <unordered_map>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::meshing;
using namespace utymap::index;

namespace {

    const std::string RoofTypeKey = "roof-type";
    const std::string RoofHeightKey = "roof-height";
    const std::string FacadeTypeKey = "facade-type";

    const std::string HeightKey = "height";
    const std::string MinHeightKey = "min-height";
    
    const std::string BuilderKey = "builders";
    const std::string BuilderName = "building";

    const std::string MeshNamePrefix = "building:";

    // Defines roof builder which does nothing.
    class EmptyRoofBuilder : public RoofBuilder {
    public:
        EmptyRoofBuilder(const BuilderContext& bc, MeshContext& mc)
            : RoofBuilder(bc, mc) { }
        void build(utymap::meshing::Polygon&) {}
    };

    typedef std::function<std::shared_ptr<RoofBuilder>(const BuilderContext&, MeshContext&)> RoofBuilderFactory;
    std::unordered_map<std::string, RoofBuilderFactory> RoofBuilderFactoryMap =
    {
        {
            "none",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<EmptyRoofBuilder>(builderContext, meshContext);
            }
        },
        { 
            "flat",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<FlatRoofBuilder>(builderContext, meshContext);
            } 
        },
        {
            "dome",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<DomeRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "pyramidal",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<PyramidalRoofBuilder>(builderContext, meshContext);
            }
        },
        {
            "mansard",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<MansardRoofBuilder>(builderContext, meshContext);
            }
        }
    };

    typedef std::function<std::shared_ptr<FacadeBuilder>(const BuilderContext&, MeshContext&)> FacadeBuilderFactory;
    std::unordered_map<std::string, FacadeBuilderFactory> FacadeBuilderFactoryMap =
    {
        {
            "flat",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<FlatFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "cylinder",
            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<CylinderFacadeBuilder>(builderContext, meshContext);
            }
        },
        {
            "sphere",

            [](const BuilderContext& builderContext, MeshContext& meshContext) {
                return std::make_shared<SphereFacadeBuilder>(builderContext, meshContext);
            }
        }

    };
}

namespace utymap { namespace builders {

class LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl : public ElementBuilder
{
public:
    LowPolyBuildingBuilderImpl(const utymap::builders::BuilderContext& context) :
                              ElementBuilder(context), mesh_()
    {
    }

    void visitNode(const utymap::entities::Node&) { }

    void visitWay(const utymap::entities::Way&) { }

    void visitArea(const utymap::entities::Area& area)
    {
        bool justCreated = ensureMesh(area);

        Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);

        // We're processing relation and this is not a building part.
        if (!justCreated && !isBuilding(style))
            return;
         
        MeshContext meshContext(*mesh_, style);

        Polygon polygon(area.coordinates.size(), 0);
        polygon.addContour(toPoints(area.coordinates));

        double height = style.getValue(HeightKey, area.tags);
        double minHeight = style.getValue(MinHeightKey, area.tags);
        double elevation = context_.eleProvider.getElevation(area.coordinates[0]) + minHeight;

        height -= minHeight;

        // roof
        auto roofType = style.getString(RoofTypeKey);
        double roofHeight = style.getValue(RoofHeightKey, area.tags);
        auto roofBuilder = RoofBuilderFactoryMap.find(*roofType)->second(context_, meshContext);
        roofBuilder->setHeight(roofHeight);
        roofBuilder->setMinHeight(elevation + height);
        roofBuilder->build(polygon);
        
        // facade
        auto facadeType = style.getString(FacadeTypeKey);
        auto facadeBuilder = FacadeBuilderFactoryMap.find(*facadeType)->second(context_, meshContext);
        facadeBuilder->setHeight(height);
        facadeBuilder->setMinHeight(elevation);
        facadeBuilder->build(polygon);       

        runMeshCallbackIfNecessary(justCreated);
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        bool justCreated = ensureMesh(relation);

        for (const auto& element : relation.elements)  {

            // TODO holes are not supported yet (inner ring).
            if (element->tags.empty())
                continue;

            element->accept(*this);
        }

        runMeshCallbackIfNecessary(justCreated);
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

    inline bool ensureMesh(const Element& element)
    {
        if (mesh_ == nullptr) {
            mesh_ = std::make_shared<Mesh>(utymap::utils::getMeshName(MeshNamePrefix, element));
            return true;
        }

        return false;
    }

    inline void runMeshCallbackIfNecessary(bool justCreated)
    {
        // NOTE should be called once when building is created.
        if (justCreated) {
            context_.meshCallback(*mesh_);
            mesh_.reset();
        }
    }

    inline bool isBuilding(const Style& style)
    {
        return style.getString(BuilderKey, "")->find(BuilderName) != std::string::npos;
    }

    std::shared_ptr<Mesh> mesh_;
};

LowPolyBuildingBuilder::LowPolyBuildingBuilder(const BuilderContext& context) 
    : ElementBuilder(context), pimpl_(new LowPolyBuildingBuilder::LowPolyBuildingBuilderImpl(context))
{
}

LowPolyBuildingBuilder::~LowPolyBuildingBuilder() { }

void LowPolyBuildingBuilder::visitNode(const Node&) { }

void LowPolyBuildingBuilder::visitWay(const Way&) { }

void LowPolyBuildingBuilder::visitArea(const Area& area)
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

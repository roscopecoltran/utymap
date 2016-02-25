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

using utymap::QuadKey;
using utymap::heightmap::ElevationProvider;
using utymap::mapcss::StyleProvider;
using utymap::meshing::Mesh;
using utymap::index::StringTable;

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
        // TODO
    }

    void visitRelation(const utymap::entities::Relation&)
    {
        // TODO
    }

private:
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

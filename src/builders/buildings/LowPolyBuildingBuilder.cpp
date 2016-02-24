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

namespace utymap { namespace builders {

LowPolyBuildingBuilder::LowPolyBuildingBuilder()
{

}

void LowPolyBuildingBuilder::visitNode(const utymap::entities::Node&)
{

}

void LowPolyBuildingBuilder::visitWay(const utymap::entities::Way&)
{

}

void LowPolyBuildingBuilder::visitArea(const utymap::entities::Area& area)
{
    // TODO
}

void LowPolyBuildingBuilder::visitRelation(const utymap::entities::Relation&)
{
    // TODO
}

}}

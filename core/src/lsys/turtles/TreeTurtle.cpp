#include "lsys/turtles/TreeTurtle.hpp"
#include <mapcss/StyleConsts.hpp>
#include "utils/GeometryUtils.hpp"

#include <functional>
#include <unordered_map>

using namespace utymap::builders;
using namespace utymap::lsys;
using namespace utymap::mapcss;

namespace {
    const std::string FoliagePrefix = "foliage-";
    const std::string FoliageGradientKey = FoliagePrefix + StyleConsts::GradientKey();
    const std::string FoliageRadius = FoliagePrefix + StyleConsts::RadiusKey();
    const std::string FoliageTextureIndexKey = FoliagePrefix + StyleConsts::TextureIndexKey();
    const std::string FoliageTextureTypeKey = FoliagePrefix + StyleConsts::TextureTypeKey();
    const std::string FoliageTextureScaleKey = FoliagePrefix + StyleConsts::TextureScaleKey();

    const std::string TrunkPrefix = "trunk-";
    const std::string TrunkGradientKey = TrunkPrefix + StyleConsts::GradientKey();
    const std::string TrunkRadius = TrunkPrefix + StyleConsts::RadiusKey();
    const std::string TrunkHeight = TrunkPrefix + StyleConsts::HeightKey();;
    const std::string TrunkTextureIndexKey = TrunkPrefix + StyleConsts::TextureIndexKey();;
    const std::string TrunkTextureTypeKey = TrunkPrefix + StyleConsts::TextureTypeKey();
    const std::string TrunkTextureScaleKey = TrunkPrefix + StyleConsts::TextureScaleKey();
}

TreeTurtle::TreeTurtle(const BuilderContext& builderContext, MeshContext& meshContext) :
    cylinderGenerator_(builderContext, meshContext),
    icoSphereGenerator_(builderContext, meshContext),
    trunkSize_(utymap::utils::getSize(builderContext.boundingBox, meshContext.style, TrunkRadius)),
    foliageSize_(utymap::utils::getSize(builderContext.boundingBox, meshContext.style, FoliageRadius))
{
    cylinderGenerator_
        .setRadius(trunkSize_);
}

void TreeTurtle::moveForward()
{

}

void TreeTurtle::say(const std::string& word)
{

}

void TreeTurtle::addLeaf()
{
    
}

void TreeTurtle::addTrunk()
{

}

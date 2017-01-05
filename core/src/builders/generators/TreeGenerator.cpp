#include "builders/generators/TreeGenerator.hpp"
#include <mapcss/StyleConsts.hpp>
#include "utils/GeometryUtils.hpp"
#include "utils/GradientUtils.hpp"

#include <functional>
#include <unordered_map>

using namespace utymap::builders;
using namespace utymap::lsys;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const std::string LeafPrefix = "leaf-";
    const std::string LeafGradientKey = LeafPrefix + StyleConsts::GradientKey();
    const std::string LeafRadius = LeafPrefix + StyleConsts::RadiusKey();
    const std::string LeafTextureIndexKey = LeafPrefix + StyleConsts::TextureIndexKey();
    const std::string LeafTextureTypeKey = LeafPrefix + StyleConsts::TextureTypeKey();
    const std::string LeafTextureScaleKey = LeafPrefix + StyleConsts::TextureScaleKey();

    const std::string TrunkPrefix = "trunk-";
    const std::string TrunkGradientKey = TrunkPrefix + StyleConsts::GradientKey();
    const std::string TrunkRadius = TrunkPrefix + StyleConsts::RadiusKey();
    const std::string TrunkHeight = TrunkPrefix + StyleConsts::HeightKey();;
    const std::string TrunkTextureIndexKey = TrunkPrefix + StyleConsts::TextureIndexKey();;
    const std::string TrunkTextureTypeKey = TrunkPrefix + StyleConsts::TextureTypeKey();
    const std::string TrunkTextureScaleKey = TrunkPrefix + StyleConsts::TextureScaleKey();
}

std::unordered_map<std::string, void(TreeGenerator::*)()> TreeGenerator::WordMap =
{
    { "cone", &TreeGenerator::addCone },
    { "leaf", &TreeGenerator::addLeaf },
    { "trunk", &TreeGenerator::addTrunk },
};

TreeGenerator::TreeGenerator(const BuilderContext& builderContext, const Style& style, Mesh& mesh) :
    builderContext_(builderContext),
    cylinderContext_(MeshContext::create(mesh, style, builderContext.styleProvider,
    TrunkGradientKey, TrunkTextureIndexKey, TrunkTextureTypeKey, TrunkTextureScaleKey)),
    icoSphereContext_(MeshContext::create(mesh, style, builderContext.styleProvider,
    LeafGradientKey, LeafTextureIndexKey, LeafTextureTypeKey, LeafTextureScaleKey)),
    cylinderGenerator_(builderContext, cylinderContext_),
    icoSphereGenerator_(builderContext, icoSphereContext_),
    trunkSize_(utymap::utils::getSize(builderContext.boundingBox, style, TrunkRadius)),
    leafSize_(utymap::utils::getSize(builderContext.boundingBox, style, LeafRadius))
{
    cylinderGenerator_
        .setMaxSegmentHeight(0)
        .setRadialSegments(7);

    icoSphereGenerator_
        .setRecursionLevel(1);
}

TreeGenerator& TreeGenerator::setPosition(const utymap::math::Vector3& position)
{
    state_.position = position;
    return *this;
}

void TreeGenerator::moveForward()
{
    addTrunk();
}

void TreeGenerator::say(const std::string& word)
{
    (this->*WordMap.at(word))();
}

void TreeGenerator::addLeaf()
{
    auto size = leafSize_ * state_.length;
    icoSphereGenerator_
        .setCenter(state_.position)
        .setSize(size)
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(icoSphereContext_.mesh,
                                                        icoSphereContext_.appearanceOptions);
    jumpForward();
}

void TreeGenerator::addTrunk()
{
    auto size = trunkSize_ * state_.length;
    cylinderGenerator_
        .setCenter(state_.position)
        .setHeight(size.y)
        .setRadius(size)
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(cylinderContext_.mesh,
                                                        cylinderContext_.appearanceOptions);
    jumpForward();
}

void TreeGenerator::addCone()
{
    auto size = trunkSize_ * state_.length;
    cylinderGenerator_
        .setCenter(state_.position)
        .setHeight(size.y)
        .setRadius(size, Vector3(0, 0, 0))
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(cylinderContext_.mesh,
                                                        cylinderContext_.appearanceOptions);
    jumpForward();
}

#include "builders/generators/TreeGenerator.hpp"
#include <mapcss/StyleConsts.hpp>
#include "utils/GeometryUtils.hpp"
#include "utils/GeoUtils.hpp"
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
    const std::string LeafTextureIndexKey = LeafPrefix + StyleConsts::TextureIndexKey();
    const std::string LeafTextureTypeKey = LeafPrefix + StyleConsts::TextureTypeKey();
    const std::string LeafTextureScaleKey = LeafPrefix + StyleConsts::TextureScaleKey();

    const std::string TrunkPrefix = "trunk-";
    const std::string TrunkGradientKey = TrunkPrefix + StyleConsts::GradientKey();
    const std::string TrunkTextureIndexKey = TrunkPrefix + StyleConsts::TextureIndexKey();;
    const std::string TrunkTextureTypeKey = TrunkPrefix + StyleConsts::TextureTypeKey();
    const std::string TrunkTextureScaleKey = TrunkPrefix + StyleConsts::TextureScaleKey();

    const std::string TreePrefix = "tree-";
    const std::string TreeHeight = TreePrefix + StyleConsts::HeightKey();
    const std::string TreeWidth = TreePrefix + StyleConsts::WidthKey();
}

std::unordered_map<std::string, void(TreeGenerator::*)()> TreeGenerator::WordMap =
{
    { "cone", &TreeGenerator::addCone },
    { "leaf", &TreeGenerator::addSphere },
    { "trunk", &TreeGenerator::addCylinder },
};

TreeGenerator::TreeGenerator(const BuilderContext& builderContext, const Style& style, Mesh& mesh) :
    builderContext_(builderContext),
    cylinderContext_(MeshContext::create(mesh, style, builderContext.styleProvider,
        TrunkGradientKey, TrunkTextureIndexKey, TrunkTextureTypeKey, TrunkTextureScaleKey)),
    icoSphereContext_(MeshContext::create(mesh, style, builderContext.styleProvider,
        LeafGradientKey, LeafTextureIndexKey, LeafTextureTypeKey, LeafTextureScaleKey)),
    cylinderGenerator_(builderContext, cylinderContext_),
    icoSphereGenerator_(builderContext, icoSphereContext_),
    translationFunc_(std::bind(&TreeGenerator::translate, this, std::placeholders::_1)),
    minHeight_(0)
{
    cylinderGenerator_
        .setMaxSegmentHeight(0)
        .setRadialSegments(7);

    icoSphereGenerator_
        .setRecursionLevel(1);

    state_.length = style.getValue(TreeHeight);
    state_.width = style.getValue(TreeWidth);
}

TreeGenerator& TreeGenerator::setPosition(const utymap::GeoCoordinate& coordinate, double height)
{
    position_ = coordinate;
    minHeight_ = height;

    cylinderGenerator_.setTranslation(translationFunc_);
    icoSphereGenerator_.setTranslation(translationFunc_);

    return *this;
}

void TreeGenerator::moveForward()
{
    addCylinder();
}

void TreeGenerator::say(const std::string& word)
{
    (this->*WordMap.at(word))();
}

void TreeGenerator::addSphere()
{
    icoSphereGenerator_
        .setCenter(state_.position)
        .setSize(Vector3(state_.width, state_.width, state_.width))
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(icoSphereContext_.mesh,
                                                        icoSphereContext_.appearanceOptions);
    jumpForward();
}

void TreeGenerator::addCylinder()
{
    cylinderGenerator_
        .setCenter(state_.position)
        .setDirection(state_.direction, state_.right)
        .setSize(Vector3(state_.width, state_.length, state_.width))
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(cylinderContext_.mesh,
                                                        cylinderContext_.appearanceOptions);
    jumpForward();
}

void TreeGenerator::addCone()
{
    cylinderGenerator_
        .setCenter(state_.position)
        .setDirection(state_.direction, state_.right)
        .setSize(Vector3(state_.width, state_.length, state_.width), Vector3(0, 0, 0))
        .generate();

    builderContext_.meshBuilder.writeTextureMappingInfo(cylinderContext_.mesh,
                                                        cylinderContext_.appearanceOptions);
    jumpForward();
}

Vector3 TreeGenerator::translate(const utymap::math::Vector3& v) const
{
    auto coordinate = GeoUtils::worldToGeo(position_, v.x, v.z);
    return Vector3(coordinate.longitude, v.y + minHeight_, coordinate.latitude);
}

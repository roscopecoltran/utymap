#include "builders/generators/TreeGenerator.hpp"
#include <mapcss/StyleConsts.hpp>
#include "utils/CoreUtils.hpp"
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

    const std::string Prefix = "lsys-";
    const std::string SizeKey = Prefix + StyleConsts::SizeKey();
    const std::string GradientsKey= Prefix + "colors";
    const std::string TextureIndicesKey = Prefix + "texture-indices";
    const std::string TextureTypesKey = Prefix + "texture-types";
    const std::string TextureScalesKey = Prefix + "texture-scales";

    /// Parses appearances from comma separated representation.
    std::vector<MeshBuilder::AppearanceOptions> createAppearances(const BuilderContext& builderContext, const Style& style)
    {
        auto colorStrings = utymap::utils::splitBy(',', style.getString(GradientsKey));
        auto indices = utymap::utils::splitBy(',', style.getString(TextureIndicesKey));
        auto types = utymap::utils::splitBy(',', style.getString(TextureTypesKey));
        auto scales = utymap::utils::splitBy(',', style.getString(TextureScalesKey));

        std::vector<MeshBuilder::AppearanceOptions> appearances;
        appearances.reserve(colorStrings.size());
        for (std::size_t i = 0; i < colorStrings.size(); ++i) {
            auto textureIndex = utymap::utils::lexicalCast<std::uint16_t>(indices.at(i));
            appearances.push_back(MeshBuilder::AppearanceOptions(
                builderContext.styleProvider.getGradient(colorStrings.at(i)),
                0,
                textureIndex,
                builderContext.styleProvider.getTexture(textureIndex, types.at(i)).random(0),
                utymap::utils::lexicalCast<double>(scales.at(i))));
        }

        return appearances;
    }

    /// Gets appearance by index safely.
    const MeshBuilder::AppearanceOptions& getAppearanceByIndex(std::size_t index, const std::vector<MeshBuilder::AppearanceOptions>& appearances)
    {
        return appearances.at(index % appearances.size());
    }
}

std::unordered_map<std::string, void(TreeGenerator::*)()> TreeGenerator::WordMap =
{
    { "cone", &TreeGenerator::addCone },
    { "sphere", &TreeGenerator::addSphere },
    { "cylinder", &TreeGenerator::addCylinder },
};

TreeGenerator::TreeGenerator(const BuilderContext& builderContext, const Style& style, Mesh& mesh) :
    builderContext_(builderContext),
    appearances_(createAppearances(builderContext, style)),
    cylinderContext_(MeshContext(mesh, style, getAppearanceByIndex(0, appearances_))),
    icoSphereContext_(MeshContext(mesh, style, getAppearanceByIndex(1, appearances_))),
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

    double size = style.getValue(SizeKey);

    state_.length = size;
    state_.width = size;
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

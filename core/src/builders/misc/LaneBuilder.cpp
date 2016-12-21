#include "builders/generators/WallGenerator.hpp"
#include "builders/misc/LaneBuilder.hpp"
#include "builders/MeshContext.hpp"
#include "entities/Way.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleConsts.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
    const std::string MeshName = "lanes";
    const std::string StylePrefix = "lane-";

    const std::string LaneWidthKey = StylePrefix + StyleConsts::WidthKey();
    const std::string LaneHeightKey = StylePrefix + StyleConsts::HeightKey();
    const std::string LaneLengthKey = StylePrefix + StyleConsts::LengthKey();

    const std::string LaneGradientKey = StylePrefix + StyleConsts::GradientKey();
    const std::string LaneTextureIndexKey = StylePrefix + StyleConsts::TextureIndexKey();
    const std::string LaneTextureTypeKey = StylePrefix + StyleConsts::TextureTypeKey();
    const std::string LaneTextureScaleKey = StylePrefix + StyleConsts::TextureScaleKey();
}

LaneBuilder::LaneBuilder(const BuilderContext& context) :
    ElementBuilder(context), mesh_(MeshName)
{
}

void LaneBuilder::visitWay(const Way& way)
{
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
    auto meshContext = MeshContext::create(mesh_, style, context_.styleProvider,
        LaneGradientKey, LaneTextureIndexKey, LaneTextureTypeKey, LaneTextureScaleKey);

    double width = style.getValue(LaneWidthKey, context_.boundingBox);
    double length = style.getValue(LaneLengthKey);

    WallGenerator generator(context_, meshContext);
    generator
        .setWidth(width)
        .setHeight(style.getValue(LaneHeightKey))
        .setGap(length)
        .setLength(length);

    for (std::size_t i = 0; i < way.coordinates.size() - 1; ++i) {
        generator
            .setGeometry(way.coordinates.begin() + i, way.coordinates.begin() + i + 2)
            .generate();
    }

    context_.meshBuilder.writeTextureMappingInfo(mesh_, meshContext.appearanceOptions);
}

void LaneBuilder::visitRelation(const Relation& relation)
{
    for (const auto& element : relation.elements)
        element->accept(*this);
}

void LaneBuilder::complete()
{
    context_.meshCallback(mesh_);
}

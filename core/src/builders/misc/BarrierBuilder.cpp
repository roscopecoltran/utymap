#include "builders/generators/WallGenerator.hpp"
#include "builders/misc/BarrierBuilder.hpp"
#include "entities/Way.hpp"
#include "utils/GradientUtils.hpp"

using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::mapcss;
using namespace utymap::math;
using namespace utymap::utils;

namespace {
    const double Scale = 1E7;
    const std::string WidthKey = "width";
    const std::string HeightKey = "height";
    const std::string LengthKey = "length";

    const std::string ColorKey = "color";
    const std::string MeshNamePrefix = "barrier:";

    const std::string TextureIndexKey = "texture-index";
    const std::string TextureTypeKey = "texture-type";
    const std::string TextureScaleKey = "texture-scale";
}

void BarrierBuilder::visitWay(const Way& way)
{
    Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
    Mesh mesh(utymap::utils::getMeshName(MeshNamePrefix, way));

    MeshContext meshContext = MeshContext::create(mesh, style, context_.styleProvider,
        ColorKey, TextureIndexKey, TextureTypeKey, TextureScaleKey, way.id);

    double width = style.getValue(WidthKey, 
        context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude,
        context_.boundingBox.center());

    WallGenerator generator(context_, meshContext);
    generator
        .setGeometry(way.coordinates)
        .setWidth(width)
        .setHeight(style.getValue(HeightKey))
        .setLength(style.getValue(LengthKey))
        .generate();

    context_.meshBuilder.writeTextureMappingInfo(mesh, meshContext.appearanceOptions);
    context_.meshCallback(mesh);
}

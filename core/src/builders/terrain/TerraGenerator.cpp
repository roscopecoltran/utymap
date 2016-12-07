#include "builders/terrain/TerraGenerator.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
    const double Scale = 1E7;
}

TerraGenerator::TerraGenerator(const utymap::builders::BuilderContext& context,
                               const utymap::mapcss::Style& style,
                               const ClipperLib::Path& tileRect,
                               const std::string& meshName) :
                               context_(context), style_(style),
                               tileRect_(tileRect),
                               mesh_(meshName),
                               rect_(context.boundingBox.minPoint.longitude,
                                     context.boundingBox.minPoint.latitude,
                                     context.boundingBox.maxPoint.longitude,
                                     context.boundingBox.maxPoint.latitude)
{
    auto size = style_.getValue(StyleConsts::GridCellSize, context_.boundingBox.height(), context_.boundingBox.center());
    splitter_.setParams(Scale, size);
}

std::vector<Vector2> TerraGenerator::restoreGeometry(const Path& geometry) const
{
    auto lastItemIndex = geometry.size() - 1;
    std::vector<utymap::math::Vector2> points;
    points.reserve(geometry.size());
    for (int i = 0; i <= lastItemIndex; i++)
        splitter_.split(geometry[i], geometry[i == lastItemIndex ? 0 : i + 1], points);

    return std::move(points);
}

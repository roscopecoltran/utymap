#ifndef BUILDERS_TERRAIN_REGIONTYPES_HPP_DEFINED
#define BUILDERS_TERRAIN_REGIONTYPES_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "builders/MeshBuilder.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleConsts.hpp"

#include <memory>

namespace utymap { namespace builders {

/// Region context encapsulates information about given region.
struct RegionContext final
{
    const utymap::mapcss::Style style;
    /// Prefix in mapcss.
    const std::string prefix;

    const utymap::builders::MeshBuilder::GeometryOptions geometryOptions;
    const utymap::builders::MeshBuilder::AppearanceOptions appearanceOptions;

    RegionContext(const utymap::mapcss::Style& style,
        const std::string& prefix,
        const utymap::builders::MeshBuilder::GeometryOptions& geometryOptions,
        const utymap::builders::MeshBuilder::AppearanceOptions& appearanceOptions) :
        style(style),
        prefix(prefix),
        geometryOptions(std::move(geometryOptions)),
        appearanceOptions(std::move(appearanceOptions))
    {
    }

    /// Creates region context using given arguments.
    static RegionContext create(const utymap::builders::BuilderContext& context,
                                             const utymap::mapcss::Style& style,
                                             const std::string& prefix)
    {
        using StyleConsts = utymap::mapcss::StyleConsts;

        double quadKeyWidth = context.boundingBox.maxPoint.latitude - context.boundingBox.minPoint.latitude;

        MeshBuilder::GeometryOptions geometryOptions(
            style.getValue(prefix + StyleConsts::MaxAreaKey, quadKeyWidth * quadKeyWidth),
            style.getValue(prefix + StyleConsts::EleNoiseFreqKey, quadKeyWidth),
            std::numeric_limits<double>::lowest(), // no fixed elevation
            style.getValue(prefix + StyleConsts::HeightOffsetKey, quadKeyWidth),
            false, // no flip
            false, // no back side
            1      // no new vertices on boundaries
            );

        auto textureIndex = static_cast<std::uint16_t>(style.getValue(prefix + StyleConsts::TextureIndexKey));
        auto textureRegion = context.styleProvider
            .getTexture(textureIndex, style.getString(prefix + StyleConsts::TextureTypeKey))
            .random(0);   // TODO use seed for randomization

        double scale = style.getValue(prefix + StyleConsts::TextureScaleKey);

        MeshBuilder::AppearanceOptions appearanceOptions(
            context.styleProvider.getGradient(style.getString(prefix + StyleConsts::GradientKey)),
            style.getValue(prefix + StyleConsts::ColorNoiseFreqKey, quadKeyWidth),
            textureIndex,
            textureRegion,
            scale > 0 ? scale : 1
            );

        return RegionContext(style, prefix, geometryOptions, appearanceOptions);
    }
};

/// Represents terrain region.
struct Region final
{
    Region() :
        level(0), area(0), context(nullptr), geometry()
    {
    }

    Region(Region&& other) :
        level(other.level),
        area(other.area),
        context(std::move(other.context)),
        geometry(std::move(other.geometry))
    {
    };

    Region(const Region&) = delete;
    Region&operator=(const Region&) = delete;
    Region&operator=(Region&&) = delete;

    /// Layer flag. If it's set all with such flag should be merged together.
    bool isLayer() const
    {
        return context == nullptr;
    }

    /// Level value: zero for objects on terrain surface.
    int level;

    /// Area of polygon.
    double area;

    /// Context is optional: might be empty if polygon is layer
    std::shared_ptr<const RegionContext> context;

    /// Geometry of region.
    ClipperLib::Paths geometry;
};

typedef std::shared_ptr<const Region> RegionPtr;
typedef std::vector<RegionPtr> Layer;
typedef std::unordered_map<std::string, Layer> Layers;

}}

#endif // BUILDERS_TERRAIN_REGIONTYPES_HPP_DEFINED

#ifndef BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/terrain/RegionTypes.hpp"
#include "builders/terrain/LineGridSplitter.hpp"

#include <memory>

namespace utymap { namespace builders {

/// Defines behaviour for generating objects on/under/above terrain.
class TerraGenerator
{
public:
    TerraGenerator(const utymap::builders::BuilderContext& context,
                   const utymap::mapcss::Style& style,
                   const ClipperLib::Path& tileRect,
                   const std::string& meshName);

    /// Called when new region is added to layer collection.
    virtual void onNewRegion(const std::string& type,
                             const utymap::entities::Element& element,
                             const utymap::mapcss::Style& style,
                             const std::shared_ptr<Region>& region) = 0;

    /// Generates mesh for given rect.
    virtual void generateFrom(Layers& layers) = 0;

    virtual ~TerraGenerator() = default;

protected:
    /// Restores geometry from clipper format.
    std::vector<utymap::math::Vector2> restoreGeometry(const ClipperLib::Path& geometry) const;

    /// Checks whether given point is on tile border.
    inline bool isOnBorder(const utymap::math::Vector2& p) const { return rect_.isOnBorder(p); }

    const utymap::builders::BuilderContext& context_;
    const utymap::mapcss::Style& style_;
    const ClipperLib::Path& tileRect_;
    utymap::math::Mesh mesh_;

private:
    const utymap::math::Rectangle rect_;
    utymap::builders::LineGridSplitter splitter_;
};

}}

#endif // BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

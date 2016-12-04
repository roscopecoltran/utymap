#ifndef BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/terrain/RegionTypes.hpp"

#include <memory>

namespace utymap { namespace builders {

/// Defines behaviour for generating objects on/under/above terrain.
class TerraGenerator
{
public:
    TerraGenerator(const utymap::builders::BuilderContext& context,
                   const utymap::mapcss::Style& style) :
        context_(context), style_(style)
    {
    }

    /// Adds region for given element.
    virtual void addRegion(const std::string& type,
                           const utymap::entities::Element& element,
                           const utymap::mapcss::Style& style,
                           std::shared_ptr<Region> region) = 0;

    /// Generates mesh for given rect.
    virtual void generate(ClipperLib::Path& tileRect) = 0;

    virtual ~TerraGenerator();

protected:
    const utymap::builders::BuilderContext& context_;
    const utymap::mapcss::Style& style_;
};

}}

#endif // BUILDERS_TERRAIN_TERRAGENERATOR_HPP_DEFINED
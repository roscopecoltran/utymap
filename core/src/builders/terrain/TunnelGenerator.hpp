#ifndef BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/RegionTypes.hpp"
#include "builders/terrain/TerraGenerator.hpp"
#include "entities/Element.hpp"

namespace utymap { namespace builders {

/// Generates meshes under terrain surface: tunnels, metro stations, etc.
class TunnelGenerator final : public TerraGenerator
{
public:
    TunnelGenerator(const BuilderContext& context,
                    const utymap::mapcss::Style& style,
                    const ClipperLib::Path& tileRect);

    /// Adds region for given element.
    void addRegion(const std::string& type, 
                   const utymap::entities::Element& element,
                   const utymap::mapcss::Style& style,
                   std::shared_ptr<Region> region) override;

    /// Generates mesh for given rect.
    void generate() override;

protected:
    void buildBackground() override;
    void addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext) override;

private:

    utymap::math::Mesh mesh_;
};

}}

#endif // BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED

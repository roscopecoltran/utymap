#ifndef BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED
#define BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED

#include "builders/terrain/TerraGenerator.hpp"

namespace utymap { namespace builders {

/// Generates meshes under terrain surface: tunnels, metro stations, etc.
class TunnelGenerator final : public TerraGenerator
{
public:
    TunnelGenerator(const BuilderContext& context,
                    const utymap::mapcss::Style& style,
                    const ClipperLib::Path& tileRect);

    /// Generates mesh for given rect.
    void generate() override;

protected:

    void onAddRegion(const std::string& type,
                     const utymap::entities::Element& element,
                     const utymap::mapcss::Style& style,
                     std::shared_ptr<Region> region) override;

    bool canHandle(std::shared_ptr<Region> region) override;

    void buildBackground() override { }

    void addGeometry(ClipperLib::Paths& geometry, const RegionContext& regionContext) override;

private:
    void addGeometry(utymap::math::Polygon& polygon, const RegionContext& regionContext);

    utymap::math::Mesh mesh_;
};

}}

#endif // BUILDERS_TERRAIN_TUNNELGENERATOR_HPP_DEFINED

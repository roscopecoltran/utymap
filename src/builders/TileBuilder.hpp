#ifndef BUILDERS_TILEBUILDER_HPP_DEFINED
#define BUILDERS_TILEBUILDER_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/ElementVisitor.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap {

// Responsible for building single tile.
class TileBuilder
{
public:

    typedef std::function<void(const utymap::meshing::Mesh&)> MeshCallback;
    typedef std::function<void(const utymap::entities::Element&)> ElementCallback;
    typedef std::function<std::shared_ptr<utymap::entities::ElementVisitor>(const QuadKey&, 
                                                                            const utymap::mapcss::StyleProvider&, 
                                                                            const MeshCallback&, 
                                                                            const ElementCallback&)> ElementVisitorFactory;

    TileBuilder(utymap::index::GeoStore& geoStore, 
                utymap::index::StringTable& stringTable,
                utymap::heightmap::ElevationProvider& eleProvider);

    ~TileBuilder();

    // Registers factory method for element builder.
    void registerElementVisitor(const std::string& name, ElementVisitorFactory factory);

    // Builds tile for given quadkey.
    void build(const utymap::QuadKey& quadKey, 
               const utymap::mapcss::StyleProvider& styleProvider,
               MeshCallback meshFunc, 
               ElementCallback elementFunc);


private:
    class TileBuilderImpl;
    std::unique_ptr<TileBuilderImpl> pimpl_;
};

}
#endif // BUILDERS_TILEBUILDER_HPP_DEFINED

#ifndef TERRAIN_TERRABUILDER_HPP_DEFINED
#define TERRAIN_TERRABUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"
#include "index/StringTable.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap { namespace builders {

// Provides the way to build tile.
class TerraBuilder : public ElementBuilder
{
public:

    TerraBuilder(utymap::index::StringTable& stringTable,
                 const utymap::mapcss::StyleProvider& styleProvider, 
                 utymap::heightmap::ElevationProvider<double>& eleProvider,
                 const std::function<void(utymap::meshing::Mesh<double>&)>& callback);

    ~TerraBuilder();

    void visitNode(const utymap::entities::Node&);

    void visitWay(const utymap::entities::Way&);

    void visitArea(const utymap::entities::Area&);

    void visitRelation(const utymap::entities::Relation&);

    void prepare(const utymap::QuadKey& quadKey);

    void complete();

private:
    class TerraBuilderImpl;
    std::unique_ptr<TerraBuilderImpl> pimpl_;
};

}}

#endif // TERRAIN_TERRABUILDER_HPP_DEFINED

#ifndef BUILDERS_TERRABUILDER_HPP_DEFINED
#define BUILDERS_TERRABUILDER_HPP_DEFINED

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

// Provides the way to terrain.
class TerraBuilder : public ElementBuilder
{
public:

    TerraBuilder(utymap::index::StringTable& stringTable,
                 const utymap::mapcss::StyleProvider& styleProvider, 
                 utymap::heightmap::ElevationProvider<double>& eleProvider,
                 std::function<void(const utymap::meshing::Mesh<double>&)> callback);

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

#endif // BUILDERS_TERRABUILDER_HPP_DEFINED

#ifndef BUILDERS_TERRABUILDER_HPP_DEFINED
#define BUILDERS_TERRABUILDER_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "heightmap/ElevationProvider.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"
#include "index/StringTable.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap { namespace builders {

// Provides the way build terrain mesh.
class TerraBuilder : public utymap::entities::ElementVisitor
{
public:

    TerraBuilder(const utymap::QuadKey& quadKey,
                 const utymap::mapcss::StyleProvider& styleProvider,
                 utymap::index::StringTable& stringTable,
                 utymap::heightmap::ElevationProvider& eleProvider,
                 std::function<void(const utymap::meshing::Mesh&)> callback);

    ~TerraBuilder();

    void visitNode(const utymap::entities::Node&);

    void visitWay(const utymap::entities::Way&);

    void visitArea(const utymap::entities::Area&);

    void visitRelation(const utymap::entities::Relation&);

    void complete();

private:
    class TerraBuilderImpl;
    std::unique_ptr<TerraBuilderImpl> pimpl_;
};

}}

#endif // BUILDERS_TERRABUILDER_HPP_DEFINED

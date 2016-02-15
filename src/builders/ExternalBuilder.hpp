#ifndef BUILDERS_EXTERNALBUILDER_HPP_DEFINED
#define BUILDERS_EXTERNALBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap { namespace builders {

// Provides the way delegate building logic to external code.
class ExternalBuilder : public ElementBuilder
{
public:

    ExternalBuilder(std::function<void(const utymap::entities::Element&)> elementCallback) :
        elementCallback_(elementCallback)
    {
    }

    void visitNode(const utymap::entities::Node& node) { elementCallback_(node); }

    void visitWay(const utymap::entities::Way& way) { elementCallback_(way); }

    void visitArea(const utymap::entities::Area& area) { elementCallback_(area); }

    void visitRelation(const utymap::entities::Relation& relation) { elementCallback_(relation); }

    void prepare(const utymap::QuadKey& quadKey,
                 const utymap::mapcss::StyleProvider& styleProvider)
    {
        // TODO implement when need
    }

    void complete()
    {
        // TODO implement when need
    }

private:
    std::function<void(const utymap::entities::Element&)> elementCallback_;
};

}}

#endif // BUILDERS_EXTERNALBUILDER_HPP_DEFINED

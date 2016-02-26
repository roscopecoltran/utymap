#ifndef BUILDERS_EXTERNALBUILDER_HPP_DEFINED
#define BUILDERS_EXTERNALBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "mapcss/StyleProvider.hpp"
#include "meshing/MeshTypes.hpp"

#include <functional>
#include <string>
#include <memory>

namespace utymap { namespace builders {

// Provides the way delegate building logic to external code.
class ExternalBuilder : public utymap::entities::ElementVisitor
{
public:

    ExternalBuilder(const utymap::builders::BuilderContext& context) :
        context_(context)
    {
    }

    void visitNode(const utymap::entities::Node& node) { context_.elementCallback(node); }

    void visitWay(const utymap::entities::Way& way) { context_.elementCallback(way); }

    void visitArea(const utymap::entities::Area& area) { context_.elementCallback(area); }

    void visitRelation(const utymap::entities::Relation& relation) { context_.elementCallback(relation); }

private:
    const utymap::builders::BuilderContext& context_;
};

}}

#endif // BUILDERS_EXTERNALBUILDER_HPP_DEFINED

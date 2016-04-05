#ifndef BUILDERS_EXTERNALBUILDER_HPP_DEFINED
#define BUILDERS_EXTERNALBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/BuilderContext.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

namespace utymap { namespace builders {

// Provides the way to delegate building logic to external code.
class ExternalBuilder : public utymap::builders::ElementBuilder
{
public:
    ExternalBuilder(const utymap::builders::BuilderContext& context) :
         utymap::builders::ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node& node) { context_.elementCallback(node); }

    void visitWay(const utymap::entities::Way& way) { context_.elementCallback(way); }

    void visitArea(const utymap::entities::Area& area) { context_.elementCallback(area); }

    void visitRelation(const utymap::entities::Relation& relation) { context_.elementCallback(relation); }

    void complete() { }
};

}}

#endif // BUILDERS_EXTERNALBUILDER_HPP_DEFINED

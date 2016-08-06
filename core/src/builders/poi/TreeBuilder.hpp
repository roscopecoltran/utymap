#ifndef BUILDERS_POI_TREEBUILDER_HPP_DEFINED
#define BUILDERS_POI_TREEBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/MeshContext.hpp"
#include "builders/generators/TreeGenerator.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

namespace utymap { namespace builders {

// Builds single tree.
class TreeBuilder : public utymap::builders::ElementBuilder
{
public:

    TreeBuilder(const utymap::builders::BuilderContext& context) :
                utymap::builders::ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node& node);

    void visitWay(const utymap::entities::Way& way);

    void visitArea(const utymap::entities::Area& area) { }

    void visitRelation(const utymap::entities::Relation& relation);

    void complete() { }

    // Creates tree generator which can be used to produce multiple trees inside mesh.
    static TreeGenerator createGenerator(const utymap::builders::BuilderContext& builderContext,
                                         utymap::builders::MeshContext& meshContext);
};

}}

#endif // BUILDERS_POI_TREEBUILDER_HPP_DEFINED

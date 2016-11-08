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
#include "math/Mesh.hpp"

namespace utymap { namespace builders {

/// Builds single tree.
class TreeBuilder final : public utymap::builders::ElementBuilder
{
public:
    explicit TreeBuilder(const utymap::builders::BuilderContext& context) :
                         utymap::builders::ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node& node) override;

    void visitWay(const utymap::entities::Way& way) override;

    void visitArea(const utymap::entities::Area& area) override { }

    void visitRelation(const utymap::entities::Relation& relation) override;

    void complete() override { }

    /// Creates tree generator which can be used to produce multiple trees inside mesh.
    static std::unique_ptr<TreeGenerator> createGenerator(const utymap::builders::BuilderContext& builderContext,
                                                          utymap::math::Mesh& mesh,
                                                          const utymap::mapcss::Style& style);
};

}}

#endif // BUILDERS_POI_TREEBUILDER_HPP_DEFINED

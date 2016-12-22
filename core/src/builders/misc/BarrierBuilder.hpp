#ifndef BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED
#define BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "builders/MeshContext.hpp"

namespace utymap { namespace builders {

/// Provides the way to build barrier.
class BarrierBuilder final : public ElementBuilder
{

public:
    explicit BarrierBuilder(const utymap::builders::BuilderContext& context) :
            ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node&) override;

    void visitArea(const utymap::entities::Area&) override;

    void visitWay(const utymap::entities::Way& way) override;

    void visitRelation(const utymap::entities::Relation&) override;

    void complete() override { }

private:
    /// Builds barrier from element.
    template <typename T>
    void build(const T& element);

    /// Builds barrier as wall.
    template <typename T>
    void buildWall(const T& element, MeshContext& meshContext);

    /// Builds barrier as pillars.
    template <typename T>
    void buildPillar(const T& element, MeshContext& meshContext);
};

}}
#endif // BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

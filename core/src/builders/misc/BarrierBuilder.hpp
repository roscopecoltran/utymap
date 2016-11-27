#ifndef BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED
#define BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"

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

    void visitRelation(const utymap::entities::Relation&) override { }

    void complete() override { }

private:
    template <typename T>
    void buildBarrier(const T& element);
};

}}
#endif // BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#ifndef BUILDERS_MISC_LANEBUILDER_HPP_DEFINED
#define BUILDERS_MISC_LANEBUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"

namespace utymap { namespace builders {

/// Provides the way to build street lights.
class LaneBuilder final : public ElementBuilder
{
public:
    explicit LaneBuilder(const utymap::builders::BuilderContext& context);

    void visitNode(const utymap::entities::Node&) override { }

    void visitArea(const utymap::entities::Area&) override { }

    void visitWay(const utymap::entities::Way& way) override;

    void visitRelation(const utymap::entities::Relation&) override;

    void complete() override;

private:
    utymap::math::Mesh mesh_;
};

} }
#endif // BUILDERS_MISC_LANEBUILDER_HPP_DEFINED

#ifndef BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED
#define BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#include "builders/ElementBuilder.hpp"
#include "mapcss/Style.hpp"
#include "meshing/Polygon.hpp"

namespace utymap { namespace builders {

// Provides the way to build barrier.
class BarrierBuilder : public ElementBuilder
{

public:
    BarrierBuilder(const utymap::builders::BuilderContext& context)
        : ElementBuilder(context)
    {
    }

    void visitNode(const utymap::entities::Node& node) { }

    void visitArea(const utymap::entities::Area& area) { }

    void visitWay(const utymap::entities::Way& way);

    void visitRelation(const utymap::entities::Relation& relation) { }

    void complete() { }
   
private:
    void buildFromPolygon(const utymap::entities::Way& way, 
                          const utymap::mapcss::Style& style,
                          utymap::meshing::Polygon& polygon);
};

}}
#endif // BUILDERS_MISC_BARRIER_BUILDER_HPP_DEFINED

#ifndef BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/ElementBuilder.hpp"

#include <memory>

namespace utymap { namespace builders {

// Responsible for building generation.
class LowPolyBuildingBuilder : public utymap::builders::ElementBuilder
{
public:
    LowPolyBuildingBuilder(const utymap::builders::BuilderContext&);

    ~LowPolyBuildingBuilder();

    void visitNode(const utymap::entities::Node&);

    void visitWay(const utymap::entities::Way&);

    void visitArea(const utymap::entities::Area& area);

    void visitRelation(const utymap::entities::Relation&);

    void complete();

private:
    class LowPolyBuildingBuilderImpl;
    std::unique_ptr<LowPolyBuildingBuilderImpl> pimpl_;
};

}}

#endif // BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED

#ifndef BUILDERS_BUILDINGS_BUILDINGBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_BUILDINGBUILDER_HPP_DEFINED

#include "builders/BuilderContext.hpp"
#include "builders/ElementBuilder.hpp"

#include <memory>

namespace utymap { namespace builders {

// Responsible for building generation.
class BuildingBuilder final: public utymap::builders::ElementBuilder
{
public:
    BuildingBuilder(const utymap::builders::BuilderContext&);

    ~BuildingBuilder();

    void visitNode(const utymap::entities::Node&) override {};

    void visitWay(const utymap::entities::Way&) override {};

    void visitArea(const utymap::entities::Area& area) override;

    void visitRelation(const utymap::entities::Relation&) override;

    void complete() override;

private:
    class BuildingBuilderImpl;
    std::unique_ptr<BuildingBuilderImpl> pimpl_;
};

}}

#endif // BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED

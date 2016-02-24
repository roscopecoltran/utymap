#ifndef BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED

#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"

namespace utymap { namespace builders {

// Responsible for building generation.
class LowPolyBuildingBuilder : public utymap::entities::ElementVisitor
{
public:
    LowPolyBuildingBuilder();

    void visitNode(const utymap::entities::Node&);

    void visitWay(const utymap::entities::Way&);

    void visitArea(const utymap::entities::Area& area);

    void visitRelation(const utymap::entities::Relation&);
};

}}

#endif // BUILDERS_BUILDINGS_LOWPOLYBUILDINGBUILDER_HPP_DEFINED

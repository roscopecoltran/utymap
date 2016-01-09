#ifndef ENTITIES_RELATION_HPP_DEFINED
#define ENTITIES_RELATION_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"

#include <vector>

namespace utymap { namespace entities {

// Represents union of elements on map.
class Relation : public Element
{
public:

    std::vector<Node> nodes;
    std::vector<Way> ways;
    std::vector<Area> areas;
    std::vector<Relation> relations;

    // Accepts visitor.
    void accept(ElementVisitor& visitor) const
    {
        visitor.visitRelation(*this);
    }
};

}}

#endif // ENTITIES_RELATION_HPP_DEFINED

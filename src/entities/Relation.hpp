#ifndef ENTITIES_RELATION_HPP_DEFINED
#define ENTITIES_RELATION_HPP_DEFINED

#include "Element.hpp"
#include "ElementVisitor.hpp"
#include "GeoCoordinate.hpp"

namespace utymap { namespace entities {

// Represents union of elements on map.
class Relation : public Element
{
public:
    // Accepts visitor.
    void accept(ElementVisitor& visitor) const
    {
        visitor.visitRelation(*this);
    }
};

}}

#endif // ENTITIES_RELATION_HPP_DEFINED

#ifndef ENTITIES_RELATION_HPP_DEFINED
#define ENTITIES_RELATION_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/ElementVisitor.hpp"

#include <vector>
#include <memory>

namespace utymap { namespace entities {

// Represents union of elements on map.
struct Relation : public Element
{
    std::vector<std::shared_ptr<Element>> elements;

    // Accepts visitor.
    void accept(ElementVisitor& visitor) const
    {
        visitor.visitRelation(*this);
    }
};

}}

#endif // ENTITIES_RELATION_HPP_DEFINED

#ifndef INDEX_ELEMENTSTORE_HPP_DEFINED
#define INDEX_ELEMENTSTORE_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "entities/Element.hpp"
#include "entities/ElementVisitor.hpp"
#include "formats/FormatTypes.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"

#include <memory>

namespace utymap { namespace index {

// Defines API to store elements.
class ElementStore
{
public:
    static const int MinLevelOfDetails = 1;
    static const int MaxLevelOfDetails = 16;

    typedef std::vector<std::vector<GeoCoordinate>> GeoPolygon;
    enum ElementType { Node, Way, Area, Relation };

    // Stores element with given polygon and tags in storage.
    bool store(const GeoPolygon& polygon,
               const utymap::formats::Tags& tags,
               const ElementStore::ElementType elementType);

    virtual ~ElementStore();

protected:
    virtual StringTable& getStringTable() const = 0;
    virtual const StyleFilter& getStyleFilter() const = 0;
    virtual utymap::entities::ElementVisitor& getElementVisitor() const = 0;

private:

    inline utymap::entities::Element* createElement(const ElementStore::ElementType elementType, 
                                                    const utymap::formats::Tags& tags) const;

    void storeInTileRange(utymap::entities::Element& element, const BoundingBox& elementBbox, int levelOfDetails);
};

}}

#endif // INDEX_ELEMENTSTORE_HPP_DEFINED

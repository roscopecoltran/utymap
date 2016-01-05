#ifndef INDEX_ELEMENTSTORE_HPP_DEFINED
#define INDEX_ELEMENTSTORE_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"
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

    ElementStore(const StyleFilter& styleFilter);

    virtual ~ElementStore();

    // Stores element in storage.
    bool store(const utymap::entities::Element& element);

protected:
    // Stores element in given quadkey.
    virtual void store(const utymap::entities::Element& element, const QuadKey& quadKey) = 0;

private:
    void storeInTileRange(const utymap::entities::Element& element,
                          const BoundingBox& elementBbox,
                          int levelOfDetails);

    const StyleFilter& styleFilter_;
};

}}

#endif // INDEX_ELEMENTSTORE_HPP_DEFINED

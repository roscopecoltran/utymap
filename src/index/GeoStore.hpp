#ifndef INDEX_GEOSTORE_HPP_DEFINED
#define INDEX_GEOSTORE_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/ElementVisitor.hpp"
#include "formats/FormatTypes.hpp"
#include "index/ElementStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"

#include "mapcss/StyleSheet.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store and access geo data.
class GeoStore
{
public:
    GeoStore(const StyleFilter& styleFilter, StringTable& stringTable);

    ~GeoStore();

    // Adds underlying element store for usage.
    void registerStore(const std::string& storeKey, ElementStore& store);

    // Adds element to selected store.
    void add(const std::string& storeKey, const utymap::entities::Element& element);

    // Adds elements from given path to selected store.
    void add(const std::string& storeKey, const std::string& path);

    // Searches for elements inside quadkey.
    void search(const QuadKey& quadKey,
                utymap::entities::ElementVisitor& visitor);

    // Searches for elements inside circle with given parameters.
    void search(const GeoCoordinate& coordinate,
                double radius,
                utymap::entities::ElementVisitor& visitor);

private:
    class GeoStoreImpl;
    std::unique_ptr<GeoStoreImpl> pimpl_;
};

}}

#endif // INDEX_GEOSTORE_HPP_DEFINED

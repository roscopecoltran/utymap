#ifndef INDEX_GEOSTORE_HPP_DEFINED
#define INDEX_GEOSTORE_HPP_DEFINED

#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "QuadKey.hpp"
#include "entities/ElementVisitor.hpp"
#include "formats/FormatTypes.hpp"
#include "index/StringTable.hpp"
#include "mapcss/StyleSheet.hpp"

#include <iostream>
#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store and access geo data.
class GeoStore
{
public:
    GeoStore(const std::string& directory,
             const utymap::mapcss::StyleSheet& stylesheet,
             StringTable& stringTable);

    ~GeoStore();

    // Searches for elements inside quadkey.
    void search(const QuadKey& quadKey,
                utymap::entities::ElementVisitor& visitor);

    // Searches for elements inside circle with given parameters.
    void search(const GeoCoordinate& coordinate,
                double radius,
                utymap::entities::ElementVisitor& visitor);

    // Saves data from stream.
    void save(std::istream& stream,
              const utymap::formats::FormatType type);

private:
    class GeoStoreImpl;
    std::unique_ptr<GeoStoreImpl> pimpl_;
};

}}

#endif // INDEX_GEOSTORE_HPP_DEFINED

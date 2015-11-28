#ifndef INDEX_GEOSTORE_HPP_DEFINED
#define INDEX_GEOSTORE_HPP_DEFINED

#include "BoundingBox.hpp"
#include "StyleFilter.hpp"
#include "entities/ElementVisitor.hpp"
#include "formats/OsmTypes.hpp"
#include "index/StringTable.hpp"

#include <iostream>
#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store and access geo data.
class GeoStore
{
public:
    GeoStore(const std::string& directory, 
            StringTable& stringTable);

    // Searches for elements using given parameters.
    void search(const BoundingBox& bbox,
                int levelOfDetails,
                utymap::entities::ElementVisitor& visitor,
                const StyleFilter& filter);

    // Imports data from stream.
    void import(std::istream& stream, 
                const utymap::formats::FormatType type,
                const StyleFilter& filter);

private:
    class GeoStoreImpl;
    std::unique_ptr<GeoStoreImpl> pimpl_;
};

}}

#endif // INDEX_GEOSTORE_HPP_DEFINED

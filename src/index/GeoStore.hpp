#ifndef INDEX_GEOSTORE_HPP_DEFINED
#define INDEX_GEOSTORE_HPP_DEFINED

#include "BoundingBox.hpp"
#include "entities/ElementVisitor.hpp"
#include "StyleFilter.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store and access geo data
class GeoStore
{
public:
    GeoStore(std::string& directory);

    // Searches for elements using given parameters
    void search(const StyleFilter& filter,
                const BoundingBox& bbox,
                int levelOfDetails,
                utymap::entities::ElementVisitor& visitor);
private:
    class GeoStoreImpl;
    std::unique_ptr<GeoStoreImpl> pimpl_;
};

}}

#endif // INDEX_GEOSTORE_HPP_DEFINED

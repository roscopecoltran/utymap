#include "GeoStore.hpp"
#include "StringTable.hpp"

using namespace utymap::entities;
using namespace utymap::index;

class GeoStore::GeoStoreImpl
{
public:

    GeoStoreImpl(const std::string& directory, StringTable& stringTable) :
        stringTable_(stringTable)
    {
    }

    void search(const BoundingBox& bbox, int levelOfDetails, const StyleFilter& filter, ElementVisitor& visitor)
    {
    }

    void import(std::istream& stream, const utymap::formats::FormatType type) 
    {
    }

private:
    StringTable& stringTable_;
};

GeoStore::GeoStore(const std::string& directory, StringTable& stringTable) :
    pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(directory, stringTable)))
{
}

void utymap::index::GeoStore::search(const BoundingBox& bbox,
                                     int levelOfDetails, 
                                     const StyleFilter& filter,
                                     ElementVisitor& visitor)
{
    pimpl_->search(bbox, levelOfDetails, filter, visitor);
}

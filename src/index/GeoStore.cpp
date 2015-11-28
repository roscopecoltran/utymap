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

    void search(const BoundingBox& bbox, int levelOfDetails, ElementVisitor& visitor, const StyleFilter& filter)
    {
    }

    void import(std::istream& stream, const utymap::formats::FormatType type, const StyleFilter& filter)
    {
    }

private:
    StringTable& stringTable_;
};

GeoStore::GeoStore(const std::string& directory, StringTable& stringTable) :
    pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(directory, stringTable)))
{
}

void utymap::index::GeoStore::search(const BoundingBox& bbox, int levelOfDetails, ElementVisitor& visitor, const StyleFilter& filter)
{
    pimpl_->search(bbox, levelOfDetails, visitor, filter);
}

void utymap::index::GeoStore::import(std::istream& stream, const utymap::formats::FormatType type, const StyleFilter& filter)
{
    pimpl_->import(stream, type, filter);
}


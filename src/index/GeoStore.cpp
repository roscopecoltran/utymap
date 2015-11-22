#include "GeoStore.hpp"
#include "StringTable.hpp"

using namespace utymap::entities;
using namespace utymap::index;

class GeoStore::GeoStoreImpl
{
public:

    GeoStoreImpl(std::string& directory) :
        stringTable_("", "")
    {
    }

    void search(const StyleFilter& filter, const BoundingBox& bbox, int levelOfDetails, ElementVisitor& visitor)
    {
    }

private:
    StringTable stringTable_;
};

GeoStore::GeoStore(std::string& directory) :
    pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(directory)))
{

}

void utymap::index::GeoStore::search(const StyleFilter& filter, 
                                     const BoundingBox& bbox, 
                                     int levelOfDetails, 
                                     ElementVisitor& visitor)
{
    pimpl_->search(filter, bbox, levelOfDetails, visitor);
}

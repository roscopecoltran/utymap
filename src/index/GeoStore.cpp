#include "index/GeoStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"
#include "mapcss/StyleSheet.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

class GeoStore::GeoStoreImpl
{
public:

    GeoStoreImpl(const std::string& directory, const StyleSheet& stylesheet, StringTable& stringTable) :
        stringTable_(stringTable),
        styleFilter_(stylesheet, stringTable)
    {
    }

    void search(const QuadKey& quadKey, utymap::entities::ElementVisitor& visitor)
    {
    }

    void import(std::istream& stream, const utymap::formats::FormatType type)
    {
    }

private:
    StringTable& stringTable_;
    StyleFilter styleFilter_;
};

GeoStore::GeoStore(const std::string& directory, const StyleSheet& stylesheet, StringTable& stringTable) :
pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(directory, stylesheet, stringTable)))
{
}

GeoStore::~GeoStore()
{
}

void utymap::index::GeoStore::search(const QuadKey& quadKey, utymap::entities::ElementVisitor& visitor)
{
    pimpl_->search(quadKey, visitor);
}

void utymap::index::GeoStore::import(std::istream& stream, const utymap::formats::FormatType type)
{
    pimpl_->import(stream, type);
}

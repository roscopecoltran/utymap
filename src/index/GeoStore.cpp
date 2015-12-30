#include "index/GeoStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"
#include "mapcss/StyleSheet.hpp"

using namespace utymap::entities;
using namespace utymap::formats;
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

    void search(const QuadKey& quadKey, ElementVisitor& visitor)
    {
    }

    void search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor)
    {
    }

    void save(std::istream& stream, const FormatType type)
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

void utymap::index::GeoStore::search(const QuadKey& quadKey, ElementVisitor& visitor)
{
    pimpl_->search(quadKey, visitor);
}

void utymap::index::GeoStore::search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor)
{
    pimpl_->search(coordinate, radius, visitor);
}

void utymap::index::GeoStore::save(std::istream& stream, const FormatType type)
{
    pimpl_->save(stream, type);
}

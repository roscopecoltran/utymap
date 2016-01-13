#include "index/LodRange.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "index/ShapeDataVisitor.hpp"

#include <unordered_map>

using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;
using namespace utymap::mapcss;

class GeoStore::GeoStoreImpl
{
public:

    GeoStoreImpl(StringTable& stringTable) :
        stringTable_(stringTable)
    {
    }

    void registerStore(const std::string& storeKey, ElementStore& store)
    {
        storeMap_[storeKey] = &store;
    }

    void add(const std::string& storeKey, const Element& element, const LodRange& range, const StyleProvider& styleProvider)
    {
    }

    void add(const std::string& storeKey, const std::string& path, const LodRange& range, const StyleProvider& styleProvider)
    {
        ElementStore* elementStorePtr = storeMap_[storeKey];
        switch (getFormatTypeFromPath(path))
        {
            case FormatType::Shape:
            {
                ShapeDataVisitor shpVisitor(*elementStorePtr, styleProvider, stringTable_, range);
                utymap::formats::ShapeParser<ShapeDataVisitor> parser;
                parser.parse(path, shpVisitor);
                break;
            }
            default:
                throw std::domain_error("Not implemented.");
        }
    }

    void search(const QuadKey& quadKey, ElementVisitor& visitor, const utymap::mapcss::StyleProvider& styleProvider)
    {
    }

    void search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor, const StyleProvider& styleProvider)
    {
    }

private:
    StringTable& stringTable_;
    std::unordered_map<std::string, ElementStore*> storeMap_;

    FormatType getFormatTypeFromPath(const std::string& path)
    {
        // TODO
        return FormatType::Shape;
    }
};

GeoStore::GeoStore(StringTable& stringTable) :
    pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(stringTable)))
{
}

GeoStore::~GeoStore()
{
}

void utymap::index::GeoStore::registerStore(const std::string& storeKey, ElementStore& store)
{
    pimpl_->registerStore(storeKey, store);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const Element& element, const LodRange& range, const StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, element, range, styleProvider);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const std::string& path, const LodRange& range, const StyleProvider& styleProvider)
{
    pimpl_->add(storeKey, path, range, styleProvider);
}

void utymap::index::GeoStore::search(const QuadKey& quadKey, ElementVisitor& visitor, const utymap::mapcss::StyleProvider& styleProvider)
{
    pimpl_->search(quadKey, visitor, styleProvider);
}

void utymap::index::GeoStore::search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor, const StyleProvider& styleProvider)
{
    pimpl_->search(coordinate, radius, visitor, styleProvider);
}

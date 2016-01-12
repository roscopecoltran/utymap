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

    GeoStoreImpl(const StyleProvider& styleProvider, StringTable& stringTable) :
        styleProvider_(styleProvider),
        stringTable_(stringTable)
    {
    }

    void registerStore(const std::string& storeKey, ElementStore& store)
    {
        storeMap_[storeKey] = &store;
    }

    void add(const std::string& storeKey, const LodRange& range, const Element& element)
    {
    }

    void add(const std::string& storeKey, const LodRange& range, const std::string& path)
    {
        ElementStore* elementStorePtr = storeMap_[storeKey];
        switch (getFormatTypeFromPath(path))
        {
            case FormatType::Shape:
            {
                ShapeDataVisitor shpVisitor(*elementStorePtr, styleProvider_, stringTable_, range);
                utymap::formats::ShapeParser<ShapeDataVisitor> parser;
                parser.parse(path, shpVisitor);
                break;
            }
            default:
                throw std::domain_error("Not implemented.");
        }
    }

    void search(const QuadKey& quadKey, ElementVisitor& visitor)
    {
    }

    void search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor)
    {
    }

private:
    const StyleProvider& styleProvider_;
    StringTable& stringTable_;

    std::unordered_map<std::string, ElementStore*> storeMap_;

    FormatType getFormatTypeFromPath(const std::string& path)
    {
        // TODO
        return FormatType::Shape;
    }
};

GeoStore::GeoStore(const StyleProvider& styleProvider, StringTable& stringTable) :
    pimpl_(std::unique_ptr<GeoStore::GeoStoreImpl>(new GeoStore::GeoStoreImpl(styleProvider, stringTable)))
{
}

GeoStore::~GeoStore()
{
}

void utymap::index::GeoStore::registerStore(const std::string& storeKey, ElementStore& store)
{
    pimpl_->registerStore(storeKey, store);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const LodRange& range, const Element& element)
{
    pimpl_->add(storeKey, range, element);
}

void utymap::index::GeoStore::add(const std::string& storeKey, const LodRange& range, const std::string& path)
{
    pimpl_->add(storeKey, range, path);
}

void utymap::index::GeoStore::search(const QuadKey& quadKey, ElementVisitor& visitor)
{
    pimpl_->search(quadKey, visitor);
}

void utymap::index::GeoStore::search(const GeoCoordinate& coordinate, double radius, ElementVisitor& visitor)
{
    pimpl_->search(coordinate, radius, visitor);
}

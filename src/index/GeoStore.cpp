#include "entities/Element.hpp"
#include "index/LodRange.hpp"
#include "formats/shape/ShapeParser.hpp"
#include "formats/xml/OsmXmlParser.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "index/ShapeDataVisitor.hpp"
#include "index/OsmDataVisitor.hpp"
#include "utils/CoreUtils.hpp"

#include <cstdint>
#include <set>
#include <stdexcept>
#include <fstream>
#include <unordered_map>

using namespace utymap::entities;
using namespace utymap::formats;
using namespace utymap::index;
using namespace utymap::mapcss;

class GeoStore::GeoStoreImpl
{
    // Prevents to visit element twice if it exists in multiply stores.
    class FilterElementVisitor : public ElementVisitor
    {
    public:
        FilterElementVisitor(ElementVisitor& visitor) : visitor_(visitor)
        {
        }

        void visitNode(const Node& node) { visitIfNecessary(node); }

        void visitWay(const Way& way)  { visitIfNecessary(way); }

        void visitArea(const Area& area)  { visitIfNecessary(area); }

        void visitRelation(const Relation& relation)  { visitIfNecessary(relation); }

    private:

        inline void visitIfNecessary(const Element& element)
        { 
            if (element.id == 0 || ids_.find(element.id) == ids_.end())
            {
                element.accept(visitor_);
                ids_.insert(element.id);
            }
        }

        ElementVisitor& visitor_;
        std::set<std::uint64_t> ids_;
    };

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
                ShapeParser<ShapeDataVisitor> parser;
                parser.parse(path, shpVisitor);
                break;
            }
            case FormatType::Xml:
            {
                OsmDataVisitor osmVisitor(*elementStorePtr, styleProvider, stringTable_, range);
                OsmXmlParser<OsmDataVisitor> parser;
                std::ifstream xmlFile(path);
                parser.parse(xmlFile, osmVisitor);
                break;
            }
            default:
                throw std::domain_error("Not implemented.");
        }
    }

    void search(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, ElementVisitor& visitor)
    {
        FilterElementVisitor filter(visitor);
        for (const auto& pair : storeMap_) {
            pair.second->search(quadKey, styleProvider, filter);
        }
    }

    void search(const GeoCoordinate& coordinate, double radius, const StyleProvider& styleProvider, ElementVisitor& visitor)
    {
        throw std::domain_error("Not implemented");
    }

private:
    StringTable& stringTable_;
    std::unordered_map<std::string, ElementStore*> storeMap_;

    FormatType getFormatTypeFromPath(const std::string& path)
    {
        if (utymap::utils::endsWith(path, "pbf"))
            return FormatType::Pbf;
        if (utymap::utils::endsWith(path, "xml"))
            return FormatType::Xml;

        return FormatType::Shape;
    }
};

GeoStore::GeoStore(StringTable& stringTable) : pimpl_(new GeoStore::GeoStoreImpl(stringTable))
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

void utymap::index::GeoStore::search(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, ElementVisitor& visitor)
{
    pimpl_->search(quadKey, styleProvider, visitor);
}

void utymap::index::GeoStore::search(const GeoCoordinate& coordinate, double radius, const StyleProvider& styleProvider, ElementVisitor& visitor)
{
    pimpl_->search(coordinate, radius, styleProvider, visitor);
}

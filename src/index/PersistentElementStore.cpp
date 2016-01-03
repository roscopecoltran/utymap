#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/ElementVisitor.hpp"
#include "index/PersistentElementStore.hpp"

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;

class PersistentElementStore::PersistentElementStoreImpl : public ElementVisitor
{
public:
    PersistentElementStoreImpl(const std::string& path, StringTable& stringTable, const StyleFilter& styleFilter) :
        path_(path),
        stringTable_(stringTable),
        styleFilter_(styleFilter)
    {
    }

    void visitNode(const utymap::entities::Node& node)
    {
    }

    void visitWay(const utymap::entities::Way& way)
    {
    }

    void visitArea(const utymap::entities::Area& area)
    {
    }

    virtual void visitRelation(const utymap::entities::Relation& relation)
    {
    }

    inline StringTable& getStringTable() const { return stringTable_; }

    inline const StyleFilter& getStyleFilter() const { return styleFilter_; }

private:
    std::string path_;
    StringTable& stringTable_;
    const StyleFilter& styleFilter_;
};

PersistentElementStore::PersistentElementStore(const std::string& path, StringTable& stringTable, const StyleFilter& styleFilter) :
    pimpl_(std::unique_ptr<PersistentElementStore::PersistentElementStoreImpl>(
        new PersistentElementStore::PersistentElementStoreImpl(path, stringTable, styleFilter)))
{
}

PersistentElementStore::~PersistentElementStore()
{
}

StringTable& PersistentElementStore::getStringTable() const
{
    return pimpl_->getStringTable();
}

const StyleFilter& PersistentElementStore::getStyleFilter() const
{
    return pimpl_->getStyleFilter();
}

utymap::entities::ElementVisitor& PersistentElementStore::getElementVisitor() const
{
    return *pimpl_;
}
#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/ElementVisitor.hpp"
#include "index/InMemoryElementStore.hpp"

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;

class InMemoryElementStore::InMemoryElementStoreImpl : public ElementVisitor
{
public:
    InMemoryElementStoreImpl(StringTable& stringTable, const StyleFilter& styleFilter) :
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

    void visitRelation(const utymap::entities::Relation& relation)
    {
    }

    inline StringTable& getStringTable() const { return stringTable_; }

    inline const StyleFilter& getStyleFilter() const { return styleFilter_; }

private:
    StringTable& stringTable_;
    const StyleFilter& styleFilter_;
};

InMemoryElementStore::InMemoryElementStore(StringTable& stringTable, const StyleFilter& styleFilter) :
    pimpl_(std::unique_ptr<InMemoryElementStore::InMemoryElementStoreImpl>(
        new InMemoryElementStore::InMemoryElementStoreImpl(stringTable, styleFilter)))
{
}

InMemoryElementStore::~InMemoryElementStore()
{
}

StringTable& InMemoryElementStore::getStringTable() const
{
    return pimpl_->getStringTable();
}

const StyleFilter& InMemoryElementStore::getStyleFilter() const
{
    return pimpl_->getStyleFilter();
}

utymap::entities::ElementVisitor& InMemoryElementStore::getElementVisitor() const
{
    return *pimpl_;
}

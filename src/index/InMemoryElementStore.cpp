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
    InMemoryElementStoreImpl(StringTable& stringTable) :
        stringTable_(stringTable)
    {
    }

    void store(const QuadKey& quadKey, const Element& element)
    {

    }

    void visitNode(const Node& node)
    {
    }

    void visitWay(const Way& way)
    {
    }

    void visitArea(const Area& area)
    {
    }

    virtual void visitRelation(const Relation& relation)
    {
    }

private:
    StringTable& stringTable_;
};

InMemoryElementStore::InMemoryElementStore(StringTable& stringTable) :
    pimpl_(std::unique_ptr<InMemoryElementStore::InMemoryElementStoreImpl>(
        new InMemoryElementStore::InMemoryElementStoreImpl(stringTable)))
{
}

InMemoryElementStore::~InMemoryElementStore()
{
}

void InMemoryElementStore::store(const QuadKey& quadKey, const Element& element)
{
    pimpl_->store(quadKey, element);
}

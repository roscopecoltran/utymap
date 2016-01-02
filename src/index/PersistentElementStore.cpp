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
    PersistentElementStoreImpl(const std::string& path, StringTable& stringTable) :
        path_(path),
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
    std::string path_;
    StringTable& stringTable_;
};

PersistentElementStore::PersistentElementStore(const std::string& path, StringTable& stringTable) :
    pimpl_(std::unique_ptr<PersistentElementStore::PersistentElementStoreImpl>(
        new PersistentElementStore::PersistentElementStoreImpl(path, stringTable)))
{
}

PersistentElementStore::~PersistentElementStore()
{
}

void PersistentElementStore::store(const QuadKey& quadKey, const Element& element)
{
    pimpl_->store(quadKey, element);
}

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
using namespace utymap::mapcss;

class PersistentElementStore::PersistentElementStoreImpl : public ElementVisitor
{
public:
    PersistentElementStoreImpl(const std::string& path) :
        path_(path),
        currentQuadKey_()
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

    void setQuadKey(const QuadKey& quadKey) { currentQuadKey_ = quadKey; }

private:
    std::string path_;
    QuadKey currentQuadKey_;
};

PersistentElementStore::PersistentElementStore(const std::string& path, StringTable& stringTable) :
    ElementStore(stringTable), pimpl_(new PersistentElementStore::PersistentElementStoreImpl(path))
{
}

PersistentElementStore::~PersistentElementStore()
{
}

void PersistentElementStore::storeImpl(const utymap::entities::Element& element, const QuadKey& quadKey)
{
    pimpl_->setQuadKey(quadKey);
    element.accept(*pimpl_);
}

void PersistentElementStore::search(const utymap::QuadKey& quadKey, const StyleProvider& styleProvider, utymap::entities::ElementVisitor& visitor)
{
    // TODO
}

bool PersistentElementStore::hasData(const utymap::QuadKey& quadKey) const
{
    // TODO
    return false;
}
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
    PersistentElementStoreImpl(const std::string& path, const StyleFilter& styleFilter) :
        path_(path),
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

    inline const StyleFilter& getStyleFilter() const { return styleFilter_; }

private:
    std::string path_;
    const StyleFilter& styleFilter_;
};

PersistentElementStore::PersistentElementStore(const std::string& path, const StyleFilter& styleFilter) :
    pimpl_(std::unique_ptr<PersistentElementStore::PersistentElementStoreImpl>(
        new PersistentElementStore::PersistentElementStoreImpl(path, styleFilter)))
{
}

PersistentElementStore::~PersistentElementStore()
{
}


const StyleFilter& PersistentElementStore::getStyleFilter() const
{
    return pimpl_->getStyleFilter();
}

utymap::entities::ElementVisitor& PersistentElementStore::getElementVisitor() const
{
    return *pimpl_;
}
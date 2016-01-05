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

    InMemoryElementStoreImpl(const StyleFilter& styleFilter) :
        styleFilter_(styleFilter),
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

    void visitRelation(const utymap::entities::Relation& relation)
    {
    }

    inline const StyleFilter& getStyleFilter() const { return styleFilter_; }

    void setQuadKey(const QuadKey& quadKey) { currentQuadKey_ = quadKey; }

private:
    const StyleFilter& styleFilter_;
    QuadKey currentQuadKey_;
};

InMemoryElementStore::InMemoryElementStore(const StyleFilter& styleFilter) :
    pimpl_(std::unique_ptr<InMemoryElementStore::InMemoryElementStoreImpl>(
        new InMemoryElementStore::InMemoryElementStoreImpl(styleFilter)))
{
}

InMemoryElementStore::~InMemoryElementStore()
{
}

const StyleFilter& InMemoryElementStore::getStyleFilter() const
{
    return pimpl_->getStyleFilter();
}

void InMemoryElementStore::store(const utymap::entities::Element& element, const QuadKey& quadKey)
{
    pimpl_->setQuadKey(quadKey);
    element.accept(*pimpl_);
}

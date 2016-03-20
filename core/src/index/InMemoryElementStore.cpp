#include "BoundingBox.hpp"
#include "GeoCoordinate.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "index/InMemoryElementStore.hpp"

#include <memory>
#include <map>

using namespace utymap;
using namespace utymap::index;
using namespace utymap::entities;
using namespace utymap::mapcss;

class InMemoryElementStore::InMemoryElementStoreImpl : public ElementVisitor
{
    struct QuadKeyComparator
    {
        bool operator() (const QuadKey& lhs, const QuadKey& rhs) const
        {
            if (lhs.levelOfDetail == rhs.levelOfDetail) {
                if (lhs.tileX == rhs.tileX) {
                    return lhs.tileY < rhs.tileY;
                }
                return lhs.tileX < rhs.tileX;
            }
            return lhs.levelOfDetail < rhs.levelOfDetail;
        }
    };

    typedef std::vector<std::shared_ptr<Element>> Elements;
    typedef std::map<QuadKey, Elements, QuadKeyComparator> ElementMap;

public:

    QuadKey currentQuadKey;

    void visitNode(const utymap::entities::Node& node)
    {
        elementsMap_[currentQuadKey].push_back(std::shared_ptr<Node>(new Node(node)));
    }

    void visitWay(const utymap::entities::Way& way)
    {
        elementsMap_[currentQuadKey].push_back(std::shared_ptr<Way>(new Way(way)));
    }

    void visitArea(const utymap::entities::Area& area)
    {
        elementsMap_[currentQuadKey].push_back(std::shared_ptr<Area>(new Area(area)));
    }

    void visitRelation(const utymap::entities::Relation& relation)
    {
        elementsMap_[currentQuadKey].push_back(std::shared_ptr<Relation>(new Relation(relation)));
    }

    ElementMap::const_iterator begin()
    {
        return elementsMap_.find(currentQuadKey);
    }

    ElementMap::const_iterator end()
    {
        return elementsMap_.cend();
    }

    bool hasData(const utymap::QuadKey& quadKey) const
    {
        return elementsMap_.find(currentQuadKey) != elementsMap_.end();
    }

private:
    ElementMap elementsMap_;
};

InMemoryElementStore::InMemoryElementStore(StringTable& stringTable) :
    ElementStore(stringTable), pimpl_(new InMemoryElementStore::InMemoryElementStoreImpl())
{
}

InMemoryElementStore::~InMemoryElementStore()
{
}

void InMemoryElementStore::storeImpl(const utymap::entities::Element& element, const QuadKey& quadKey)
{
    pimpl_->currentQuadKey = quadKey;
    element.accept(*pimpl_);
}

bool InMemoryElementStore::hasData(const utymap::QuadKey& quadKey) const
{
    return pimpl_->hasData(quadKey);
}

void InMemoryElementStore::search(const utymap::QuadKey& quadKey, const StyleProvider& styleProvider, utymap::entities::ElementVisitor& visitor)
{
    pimpl_->currentQuadKey = quadKey;
    auto it = pimpl_->begin();

    // No elements for this quadkey
    if (it == pimpl_->end())
        return;

    for (const auto& element : it->second) {
        if (styleProvider.hasStyle(*element, quadKey.levelOfDetail))
            element->accept(visitor);
    }
}

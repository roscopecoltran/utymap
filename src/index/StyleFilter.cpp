#include "../entities/ElementVisitor.hpp"
#include "StyleFilter.hpp"
#include "StringTable.hpp"

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

class StyleElementVisitor : public ElementVisitor
{
public:

    StyleElementVisitor(int levelOfDetails) :
        levelOfDetails_(levelOfDetails),
        isApplicable_(false)
    {
    }

    void visitNode(const Node&)
    {

    }
    void visitWay(const Way&)
    {

    }
    void visitRelation(const Relation&)
    {

    }

    inline bool isApplicable() { return isApplicable_; }

private:
    int levelOfDetails_;
    bool isApplicable_;
};

class StyleFilter::StyleFilterImpl
{
public:

    StyleFilterImpl(const StyleSheet& stylesheet, StringTable& stringTable) :
        stylesheet_(stylesheet),
        stringTable_(stringTable)
    {
    }

    bool isApplicable(const Element& element, int levelOfDetails) const
    {
        StyleElementVisitor visitor = { levelOfDetails };
        element.accept(visitor);
        return visitor.isApplicable();
    }

private:
    const StyleSheet& stylesheet_;
    StringTable& stringTable_;
};

StyleFilter::StyleFilter(const StyleSheet& stylesheet, StringTable& stringTable) :
    pimpl_(std::unique_ptr<StyleFilter::StyleFilterImpl>(
        new StyleFilter::StyleFilterImpl(stylesheet, stringTable)))
{
}

bool utymap::index::StyleFilter::isApplicable(const Element& element, int levelOfDetails) const
{
    return pimpl_->isApplicable(element, levelOfDetails);
}

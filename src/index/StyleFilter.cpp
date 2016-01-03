#include "entities/ElementVisitor.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include "StyleFilter.hpp"
#include "StringTable.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

// Contains operation types supported by mapcss parser.
enum OpType { Exists, Equals, NotEquals };

struct Condition
{
    uint32_t key;
    uint32_t value;
    OpType type;
};

struct Filter
{
    std::vector<::Condition> conditions;
};

// key: level of detais, value: filters for specific element type.
typedef std::unordered_map<int, std::vector<Filter>> FilterMap;

struct FilterCollection
{
    FilterMap nodes;
    FilterMap ways;
    FilterMap areas;
    FilterMap canvases;
};

class StyleElementVisitor : public ElementVisitor
{
    typedef std::vector<Tag>::const_iterator TagIterator;
public:

    StyleElementVisitor(const FilterCollection& filters, int levelOfDetails) :
        filters_(filters),
        levelOfDetails_(levelOfDetails),
        isApplicable_(false)
    {
    }

    inline void visitNode(const Node& node)
    {
        isApplicable_ = check(node.tags, filters_.nodes);
    }

    inline void visitWay(const Way& way)
    {
        isApplicable_ = check(way.tags, filters_.ways);
    }

    inline void visitArea(const Area& area)
    {
        isApplicable_ = check(area.tags, filters_.areas);
    }

    inline void visitRelation(const Relation&)
    {
    }

    // checks tag's value assuming that the key is already checked.
    inline bool match_tag(const Tag& tag, const ::Condition& condition)
    {
        switch (condition.type)
        {
            case OpType::Exists: return true;
            case OpType::Equals: return tag.value == condition.value;
            case OpType::NotEquals: return tag.value != condition.value;
        }
        return false;
    }

    // tries to find tag which satisfy condition using binary search.
    inline bool match_tags(TagIterator begin, TagIterator end, const ::Condition& condition)
    {
        while (begin < end)
        {
            const TagIterator middle = begin + (std::distance(begin, end) / 2);
            if (middle->key == condition.key)
                return match_tag(*middle, condition);
            else if (middle->key > condition.key)
                end = middle;
            else
                begin = middle + 1;
        }
        return false;
    }

    inline bool check(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                bool isMatched = true;
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                    isMatched &= match_tags(tags.cbegin(), tags.cend(), *it);
                }
                if (isMatched) return true;
            }
        }
        return false;
    }

    inline bool isApplicable() { return isApplicable_; }

private:
    const FilterCollection& filters_;
    int levelOfDetails_;
    bool isApplicable_;
};

// Converts mapcss stylesheet to index optimized representation to speed search query up.
class StyleFilter::StyleFilterImpl
{
public:

    FilterCollection filters;

    StyleFilterImpl(const StyleSheet& stylesheet, StringTable& stringTable) :
        stringTable_(stringTable),
        filters()
    {
        filters.nodes.reserve(24);
        filters.ways.reserve(24);
        filters.areas.reserve(24);
        filters.canvases.reserve(24);

        for (const Rule& rule : stylesheet.rules) {
            for (const Selector& selector : rule.selectors) {
                FilterMap* filtersPtr;
                if (selector.name == "node") filtersPtr = &filters.nodes;
                else if (selector.name == "way") filtersPtr = &filters.ways;
                else if (selector.name == "area") filtersPtr = &filters.areas;
                else if (selector.name == "canvas") filtersPtr = &filters.canvases;
                else
                    std::domain_error("Unexpected selector name:" + selector.name);

                Filter filter;
                filter.conditions.reserve(selector.conditions.size());
                for (const utymap::mapcss::Condition& condition : selector.conditions) {
                    ::Condition c;
                    if (condition.operation == "") c.type = OpType::Exists;
                    else if (condition.operation == "=") c.type = OpType::Equals;
                    else if (condition.operation == "!=") c.type = OpType::NotEquals;
                    else
                        std::domain_error("Unexpected condition operation:" + condition.operation);

                    c.key = stringTable_.getId(condition.key);
                    c.value = stringTable_.getId(condition.value);
                    filter.conditions.push_back(c);
                }

                std::sort(filter.conditions.begin(), filter.conditions.end(),
                    [](const ::Condition& c1, const ::Condition& c2) { return c1.key > c2.key; });
                for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
                    (*filtersPtr)[i].push_back(filter);
                }
            }
        }
    }

private:

    StringTable& stringTable_;
};

StyleFilter::StyleFilter(const StyleSheet& stylesheet, StringTable& stringTable) :
    pimpl_(std::unique_ptr<StyleFilter::StyleFilterImpl>(
        new StyleFilter::StyleFilterImpl(stylesheet, stringTable)))
{
}

StyleFilter::~StyleFilter()
{
}

inline bool utymap::index::StyleFilter::isApplicable(const Element& element, int levelOfDetails) const
{
    StyleElementVisitor visitor = { pimpl_->filters, levelOfDetails };
    element.accept(visitor);
    return visitor.isApplicable();
}

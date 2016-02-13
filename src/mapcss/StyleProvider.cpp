#include "entities/ElementVisitor.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/StringTable.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/GradientUtils.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
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
    std::unordered_map<uint32_t, std::shared_ptr<std::string>> declarations;
};

// key: level of detais, value: filters for specific element type.
typedef std::unordered_map<int, std::vector<Filter>> FilterMap;

struct FilterCollection
{
    FilterMap nodes;
    FilterMap ways;
    FilterMap areas;
    FilterMap relations;
    FilterMap canvases;
};

class StyleBuilder : public ElementVisitor
{
    typedef std::vector<Tag>::const_iterator TagIterator;
public:

    StyleBuilder(const FilterCollection& filters, int levelOfDetails, bool onlyCheck = false) :
        filters_(filters),
        levelOfDetails_(levelOfDetails),
        onlyCheck_(onlyCheck),
        canBuild_(false),
        style_()
    {
    }

    void visitNode(const Node& node)
    {
        if (onlyCheck_)
            check(node.tags, filters_.nodes);
        else
            build(node.tags, filters_.nodes);
    }

    void visitWay(const Way& way)
    {
        if (onlyCheck_)
            check(way.tags, filters_.ways);
        else
            build(way.tags, filters_.ways);
    }

    void visitArea(const Area& area)
    {
        if (onlyCheck_)
            check(area.tags, filters_.areas);
        else
            build(area.tags, filters_.areas);
    }

    void visitRelation(const Relation& relation)
    {
        if (onlyCheck_)
            check(relation.tags, filters_.relations);
        else
            build(relation.tags, filters_.relations);
    }

    inline Style build()
    {
        if (onlyCheck_)
            throw std::domain_error("Cannot build feature for check mode.");

        return style_;
    }

    inline bool canBuild() { return canBuild_; }

private:

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

    // Builds style object. More expensive to call than check.
    inline void build(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                for(auto it = filter.conditions.cbegin(); it != filter.conditions.cend(); ++it) {
                    bool isMatched = true;
                    for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                        isMatched &= match_tags(tags.cbegin(), tags.cend(), *it);
                    }
                    // merge declarations to style
                    if (isMatched) {
                        canBuild_ = true;
                        for (const auto& d : filter.declarations) {
                            style_.put(d.first, d.second);
                        }
                    }
                }
            }
        }
    }

    // Just checks whether style can be created without constructing actual style.
    inline void check(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend(); ++it) {
                    bool isMatched = true;
                    for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                        isMatched &= match_tags(tags.cbegin(), tags.cend(), *it);
                    }
                    if (isMatched) {
                        canBuild_ = true;
                        return;
                    }
                }
            }
        }
    }

    const FilterCollection& filters_;
    int levelOfDetails_;
    bool onlyCheck_;
    bool canBuild_;
    Style style_;
};

// Converts mapcss stylesheet to index optimized representation to speed search query up.
class StyleProvider::StyleProviderImpl
{
public:

    FilterCollection filters;
    StringTable& stringTable;

    StyleProviderImpl(const StyleSheet& stylesheet, StringTable& stringTable) :
        stringTable(stringTable),
        filters(),
        gradients_()
    {
        filters.nodes.reserve(24);
        filters.ways.reserve(24);
        filters.areas.reserve(24);
        filters.relations.reserve(24);
        filters.canvases.reserve(24);

        for (const Rule& rule : stylesheet.rules) {
            for (const Selector& selector : rule.selectors) {
                FilterMap* filtersPtr;
                if (selector.name == "node") filtersPtr = &filters.nodes;
                else if (selector.name == "way") filtersPtr = &filters.ways;
                else if (selector.name == "area") filtersPtr = &filters.areas;
                else if (selector.name == "relation") filtersPtr = &filters.relations;
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

                    c.key = stringTable.getId(condition.key);
                    c.value = stringTable.getId(condition.value);
                    filter.conditions.push_back(c);
                }

                filter.declarations.reserve(rule.declarations.size());
                for (auto i = 0; i < rule.declarations.size(); ++i) {
                    Declaration declaration = rule.declarations[i];
                    uint32_t key = stringTable.getId(declaration.key);
                    filter.declarations[key] = std::shared_ptr<std::string>(new std::string(declaration.value));
                }

                std::sort(filter.conditions.begin(), filter.conditions.end(),
                    [](const ::Condition& c1, const ::Condition& c2) { return c1.key > c2.key; });
                for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
                    (*filtersPtr)[i].push_back(filter);
                }
            }
        }
    }

    const ColorGradient& getGradient(const std::string& key)
    {
        // TODO make thread safe
        if (gradients_.find(key) == gradients_.end()) {
            gradients_[key] = utymap::utils::GradientUtils::parseGradient(key);
        }

        return gradients_[key];
    }

private:
    std::unordered_map<std::string, ColorGradient> gradients_;
};

StyleProvider::StyleProvider(const StyleSheet& stylesheet, StringTable& stringTable) :
    pimpl_(std::unique_ptr<StyleProvider::StyleProviderImpl>(new StyleProvider::StyleProviderImpl(stylesheet, stringTable)))
{
}

StyleProvider::~StyleProvider()
{
}

bool StyleProvider::hasStyle(const utymap::entities::Element& element, int levelOfDetails) const
{
    StyleBuilder builder = { pimpl_->filters, levelOfDetails, true };
    element.accept(builder);
    return builder.canBuild();
}

Style StyleProvider::forElement(const Element& element, int levelOfDetails) const
{
    StyleBuilder builder = { pimpl_->filters, levelOfDetails };
    element.accept(builder);
    return std::move(builder.build());
}

Style StyleProvider::forCanvas(int levelOfDetails) const
{
    Style style;
    for (const auto& rule : pimpl_->filters.canvases[levelOfDetails]) {
        for (const auto& filter : pimpl_->filters.canvases[levelOfDetails]) {
            for (const auto& declaration : filter.declarations) {
                style.put(declaration.first, declaration.second);
            }
        }
    }
    return std::move(style);
}

const ColorGradient& StyleProvider::getGradient(const std::string& key) const
{
    return pimpl_->getGradient(key);
}
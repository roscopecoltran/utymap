#include "entities/ElementVisitor.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "mapcss/Style.hpp"
#include "mapcss/StyleProvider.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GradientUtils.hpp"

#include <functional>
#include <mutex>

using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;

namespace {

const std::string DefaultTextureName = "";

// Contains operation types supported by mapcss parser.
enum class OpType { Exists, Equals, NotEquals, Less, Greater };

struct ConditionType final
{
    uint32_t key;
    uint32_t value;
    OpType type;
};

struct Filter final
{
    std::vector<ConditionType> conditions;
    std::unordered_map<uint32_t, std::unique_ptr<const StyleDeclaration>> declarations;

    Filter() = default;
    Filter(const Filter& other) = delete;

    Filter(Filter&& other) :
        conditions(std::move(other.conditions)),
        declarations(std::move(other.declarations))
    {
    }
};

// key: level of details, value: filters for specific element type.
typedef std::unordered_map<int, std::vector<Filter>> FilterMap;

struct FilterCollection final
{
    FilterMap nodes;
    FilterMap ways;
    FilterMap areas;
    FilterMap relations;
    FilterMap canvases;
};

class StyleBuilder final : public ElementVisitor
{
    typedef std::vector<Tag>::const_iterator TagIterator;
public:

    StyleBuilder(std::vector<Tag> tags, StringTable& stringTable,
                 const FilterCollection& filters, int levelOfDetails, bool onlyCheck = false) :
            style(tags, stringTable),
            filters_(filters),
            levelOfDetails_(levelOfDetails),
            onlyCheck_(onlyCheck),
            canBuild_(false),
            stringTable_(stringTable)
    {
    }

    void visitNode(const Node& node) override { checkOrBuild(node.tags, filters_.nodes); }

    void visitWay(const Way& way) override { checkOrBuild(way.tags, filters_.ways); }

    void visitArea(const Area& area) override { checkOrBuild(area.tags, filters_.areas); }

    void visitRelation(const Relation& relation) override { checkOrBuild(relation.tags, filters_.relations); }

    bool canBuild() const { return canBuild_; }

    Style style;

private:

    void checkOrBuild(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        if (onlyCheck_)
            check(tags, filters);
        else
            build(tags, filters);
    }

    // checks tag's value assuming that the key is already checked.
    bool matchTag(const Tag& tag, const ConditionType& condition)
    {
        switch (condition.type) {
            case OpType::Exists:
                return true;
            case OpType::Equals:
                return tag.value == condition.value;
            case OpType::NotEquals:
                return tag.value != condition.value;
            case OpType::Less:
                return compareDoubles(tag.value, condition.value, std::less<double>());
            case OpType::Greater:
                return compareDoubles(tag.value, condition.value, std::greater<double>());
        }
    }

    // Compares two raw string values using double conversion.
    template<typename Func>
    bool compareDoubles(std::uint32_t left, std::uint32_t right, Func binaryOp)
    {
        double leftValue = utymap::utils::parseDouble(stringTable_.getString(left));
        double rightValue = utymap::utils::parseDouble(stringTable_.getString(right));

        return binaryOp(leftValue, rightValue);
    }

    // tries to find tag which satisfy condition using binary search.
    bool matchTags(TagIterator begin, TagIterator end, const ConditionType& condition)
    {
        while (begin < end) {
            const TagIterator middle = begin + (std::distance(begin, end) / 2);
            if (middle->key == condition.key)
                return matchTag(*middle, condition);
            else if (middle->key > condition.key)
                end = middle;
            else
                begin = middle + 1;
        }
        return false;
    }

    // Builds style object. More expensive to call than check.
    void build(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                bool isMatched = true;
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                    isMatched &= matchTags(tags.cbegin(), tags.cend(), *it);
                }
                // merge declarations to style
                if (isMatched) {
                    canBuild_ = true;
                    for (const auto& d : filter.declarations) {
                        style.put(*d.second);
                    }
                }
            }
        }
    }

    // Just checks whether style can be created without constructing actual style.
    void check(const std::vector<Tag>& tags, const FilterMap& filters)
    {
        FilterMap::const_iterator iter = filters.find(levelOfDetails_);
        if (iter != filters.end()) {
            for (const Filter& filter : iter->second) {
                bool isMatched = true;
                for (auto it = filter.conditions.cbegin(); it != filter.conditions.cend() && isMatched; ++it) {
                    isMatched &= matchTags(tags.cbegin(), tags.cend(), *it);
                }
                if (isMatched) {
                    canBuild_ = true;
                    return;
                }
            }
        }
    }

    const FilterCollection &filters_;
    int levelOfDetails_;
    bool onlyCheck_;
    bool canBuild_;
    StringTable& stringTable_;
};

}

// Converts mapcss stylesheet to index optimized representation to speed search query up.
class StyleProvider::StyleProviderImpl
{
public:

    FilterCollection filters;
    StringTable& stringTable;

    std::unordered_map<std::string, std::unique_ptr<const ColorGradient>> gradients;
    std::unordered_map<std::string, std::unique_ptr<const TextureAtlas>> textures;

    StyleProviderImpl(const StyleSheet& stylesheet, 
                      StringTable& stringTable) :
        filters(),
        stringTable(stringTable),
        gradients(),
        textures()
    {
        filters.nodes.reserve(24);
        filters.ways.reserve(24);
        filters.areas.reserve(24);
        filters.relations.reserve(24);
        filters.canvases.reserve(24);

        for (const Rule& rule : stylesheet.rules) {
            for (const Selector& selector : rule.selectors) {
                for (const std::string& name : selector.names) {
                    FilterMap* filtersPtr = nullptr;
                    if (name == "node") filtersPtr = &filters.nodes;
                    else if (name == "way") filtersPtr = &filters.ways;
                    else if (name == "area") filtersPtr = &filters.areas;
                    else if (name == "relation") filtersPtr = &filters.relations;
                    else if (name == "canvas") filtersPtr = &filters.canvases;
                    else
                        throw std::domain_error("Unexpected selector name:" + name);

                    Filter filter;
                    filter.conditions.reserve(selector.conditions.size());
                    for (const Condition& condition : selector.conditions) {
                        ConditionType c;
                        if (condition.operation == "") c.type = OpType::Exists;
                        else if (condition.operation == "=") c.type = OpType::Equals;
                        else if (condition.operation == "!=") c.type = OpType::NotEquals;
                        else if (condition.operation == "<") c.type = OpType::Less;
                        else if (condition.operation == ">") c.type = OpType::Greater;
                        else
                            throw std::domain_error("Unexpected condition operation:" + condition.operation);

                        c.key = stringTable.getId(condition.key);
                        c.value = stringTable.getId(condition.value);
                        filter.conditions.push_back(c);
                    }

                    filter.declarations.reserve(rule.declarations.size());
                    for (auto i = 0; i < rule.declarations.size(); ++i) {
                        Declaration declaration = rule.declarations[i];
                        uint32_t key = stringTable.getId(declaration.key);

                        if (utymap::utils::GradientUtils::isGradient(declaration.value))
                            addGradient(declaration.value);

                        filter.declarations[key] = utymap::utils::make_unique<const StyleDeclaration>(key, declaration.value);
                    }

                    std::sort(filter.conditions.begin(), filter.conditions.end(),
                        [](const ConditionType& c1, const ConditionType& c2) { return c1.key > c2.key; });
                    for (int i = selector.zoom.start; i <= selector.zoom.end; ++i) {
                        (*filtersPtr)[i].push_back(std::move(filter));
                    }
                }
            }
        }

        textures.emplace(DefaultTextureName, utymap::utils::make_unique<const TextureAtlas>());
        for (const auto& texture: stylesheet.textures) {
            textures.emplace(texture.name(), utymap::utils::make_unique<const TextureAtlas>(texture));
        }
    }

    const ColorGradient& getGradient(const std::string& key)
    {
        auto gradientPair = gradients.find(key);
        if (gradientPair == gradients.end()) {
            auto gradient = utymap::utils::GradientUtils::parseGradient(key);
            if (gradient->empty())
                throw MapCssException("Invalid gradient: " + key);
            std::lock_guard<std::mutex> lock(lock_);
            gradients.emplace(key, std::move(gradient));
            gradientPair = gradients.find(key);
        }
        return *gradientPair->second;
    }

    const TextureGroup& getTexture(const std::string& texture, const std::string& key) const
    {
        auto texturePair = textures.find(texture);
        if (texturePair == textures.end()) {
            texturePair = textures.find(DefaultTextureName);
        }

        return texturePair->second->get(key);
    }

private:

    void addGradient(const std::string& key)
    {
        if (gradients.find(key) == gradients.end()) {
            auto gradient = utymap::utils::GradientUtils::parseGradient(key);
            if (!gradient->empty())
                gradients.emplace(key, std::move(gradient));
        }
    }

    std::mutex lock_;
};

StyleProvider::StyleProvider(const StyleSheet& stylesheet, StringTable& stringTable) :
    pimpl_(utymap::utils::make_unique<StyleProviderImpl>(stylesheet, stringTable))
{
}

StyleProvider::~StyleProvider()
{
}

StyleProvider::StyleProvider(StyleProvider&& other) : pimpl_(std::move(other.pimpl_))
{
}

bool StyleProvider::hasStyle(const utymap::entities::Element& element, int levelOfDetails) const
{
    StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails, true);
    element.accept(builder);
    return builder.canBuild();
}

Style StyleProvider::forElement(const Element& element, int levelOfDetails) const
{
    StyleBuilder builder(element.tags, pimpl_->stringTable, pimpl_->filters, levelOfDetails);
    element.accept(builder);
    return std::move(builder.style);
}

Style StyleProvider::forCanvas(int levelOfDetails) const
{
    Style style({}, pimpl_->stringTable);
    for (const auto &filter : pimpl_->filters.canvases[levelOfDetails]) {
        for (const auto &declaration : filter.declarations) {
            style.put(*declaration.second);
        }
    }
    return std::move(style);
}

const ColorGradient& StyleProvider::getGradient(const std::string& key) const
{
    return pimpl_->getGradient(key);
}

const TextureGroup& StyleProvider::getTexture(const std::string& texture, const std::string& key) const
{
    return pimpl_->getTexture(texture, key);
}

#include "Exceptions.hpp"
#include "builders/ElementBuilder.hpp"
#include "builders/TileBuilder.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/GeoUtils.hpp"
#include "utils/CompatibilityUtils.hpp"
#include "utils/CoreUtils.hpp"

#include <stdexcept>
#include <sstream>
#include <unordered_map>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const std::string BuilderKeyName = "builders";

class TileBuilder::TileBuilderImpl
{
private:

    typedef std::unordered_map<std::string, ElementBuilderFactory> BuilderFactoryMap;

class AggregateElementBuilder : public ElementBuilder
{
public:
    AggregateElementBuilder(BuilderFactoryMap& builderFactoryMap, std::uint32_t builderKeyId,
                const MeshCallback& meshFunc, const ElementCallback& elementFunc) :
        builderFactoryMap_(builderFactoryMap),
        builderKeyId_(builderKeyId),
        meshFunc_(meshFunc),
        elementFunc_(elementFunc),
        styleProviderPtr_(nullptr),
        quadKey_()
    {
    }

    void prepare(const utymap::QuadKey& quadKey, const StyleProvider& styleProvider) 
    { 
        quadKey_ = quadKey; 
        styleProviderPtr_ = &styleProvider;
    }

    void visitNode(const Node& node) { buildElement(node); }

    void visitWay(const Way& way) { buildElement(way); }

    void visitArea(const Area& area) { buildElement(area); }

    void visitRelation(const Relation& relation) { buildElement(relation); }

    void complete()
    {
        for (const auto& pair : builders_) {
            pair.second->complete();
        }
    }

private:

    // Calls appropriate builders for given element
    void buildElement(const Element& element)
    {
        Style style = styleProviderPtr_->forElement(element, quadKey_.levelOfDetail);
        std::stringstream ss(style.get(builderKeyId_));
        while (ss.good())
        {
            std::string name;
            getline(ss, name, ',');
            element.accept(getBuilder(name));
        }
    }

    // Gets element builder which is ready for element visiting
    ElementBuilder& getBuilder(const std::string& name)
    {
        auto builderPair = builders_.find(name);
        if (builderPair != builders_.end()) {
            return *builderPair->second;
        }

        auto factory = builderFactoryMap_.find(name);
        if (factory == builderFactoryMap_.end()) {
            throw std::domain_error("Unknown element builder");
        }

        auto builder = factory->second(meshFunc_, elementFunc_);
        builders_[name] = builder;
        builder->prepare(quadKey_, *styleProviderPtr_);
        return *builder;
    }

    BuilderFactoryMap& builderFactoryMap_;
    const StyleProvider* styleProviderPtr_;
    const MeshCallback& meshFunc_;
    const ElementCallback& elementFunc_;
    utymap::QuadKey quadKey_;
    std::uint32_t builderKeyId_;
    std::unordered_map<std::string, std::shared_ptr<ElementBuilder>> builders_;
};

public:

    TileBuilderImpl(GeoStore& geoStore, std::uint32_t builderKeyId) :
        geoStore_(geoStore),
        builderKeyId_(builderKeyId)
    {
    }

    void registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
    {
        builderFactory_[name] = factory;
    }

    void build(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, const MeshCallback& meshFunc, const ElementCallback& elementFunc)
    {
        AggregateElementBuilder elementVisitor(builderFactory_, builderKeyId_, meshFunc, elementFunc);
        elementVisitor.prepare(quadKey, styleProvider);
        geoStore_.search(quadKey, styleProvider, elementVisitor);
        elementVisitor.complete();
    }

private:

    GeoStore& geoStore_;
    std::uint32_t builderKeyId_;
    BuilderFactoryMap builderFactory_;
};

void  TileBuilder::registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
{
    pimpl_->registerElementBuilder(name, factory);
}

void TileBuilder::build(const QuadKey& quadKey, const utymap::mapcss::StyleProvider& styleProvider, MeshCallback meshFunc, ElementCallback elementFunc)
{
    pimpl_->build(quadKey, styleProvider, meshFunc, elementFunc);
}

TileBuilder::TileBuilder(GeoStore& geoStore, StringTable& stringTable) :
    pimpl_(std::unique_ptr<TileBuilder::TileBuilderImpl>(new TileBuilder::TileBuilderImpl(geoStore, stringTable.getId(BuilderKeyName))))
{
}

TileBuilder::~TileBuilder()
{
}

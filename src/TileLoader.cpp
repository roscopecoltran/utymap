#include "Exceptions.hpp"
#include "TileLoader.hpp"
#include "builders/ElementBuilder.hpp"
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

class TileLoader::TileLoaderImpl
{
private:

    typedef std::unordered_map<std::string, ElementBuilderFactory> BuilderFactoryMap;

class AggregateElementBuilder : public ElementBuilder
{
public:
    AggregateElementBuilder(const StyleProvider& styleProvider, BuilderFactoryMap& builderFactoryMap, std::uint32_t builderKeyId,
        const MeshCallback& meshFunc, const ElementCallback& elementFunc) :
        styleProvider_(styleProvider),
        builderFactoryMap_(builderFactoryMap),
        builderKeyId_(builderKeyId),
        meshFunc_(meshFunc),
        elementFunc_(elementFunc),
        quadKey_()
    {
    }

    void prepare(const utymap::QuadKey& quadKey) { quadKey_ = quadKey; }

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
        Style style = styleProvider_.forElement(element, quadKey_.levelOfDetail);
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
        builder->prepare(quadKey_);
        return *builder;
    }

    BuilderFactoryMap& builderFactoryMap_;
    const StyleProvider& styleProvider_;
    const MeshCallback& meshFunc_;
    const ElementCallback& elementFunc_;
    utymap::QuadKey quadKey_;
    std::uint32_t builderKeyId_;
    std::unordered_map<std::string, std::shared_ptr<ElementBuilder>> builders_;
};

public:

    TileLoaderImpl(GeoStore& geoStore, const StyleProvider& styleProvider, std::uint32_t builderKeyId) :
        geoStore_(geoStore),
        styleProvider_(styleProvider),
        builderKeyId_(builderKeyId)
    {
    }

    void registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
    {
        builderFactory_[name] = factory;
    }

    void loadTile(const QuadKey& quadKey, const MeshCallback& meshFunc, const ElementCallback& elementFunc)
    {
        AggregateElementBuilder elementVisitor(styleProvider_, builderFactory_, builderKeyId_, meshFunc, elementFunc);
        elementVisitor.prepare(quadKey);
        geoStore_.search(quadKey, styleProvider_, elementVisitor);
        elementVisitor.complete();
    }

private:

    GeoStore& geoStore_;
    const StyleProvider& styleProvider_;
    std::uint32_t builderKeyId_;
    BuilderFactoryMap builderFactory_;
};

void  TileLoader::registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
{
    pimpl_->registerElementBuilder(name, factory);
}

void TileLoader::loadTile(const QuadKey& quadKey, MeshCallback meshFunc, ElementCallback elementFunc)
{
    pimpl_->loadTile(quadKey, meshFunc, elementFunc);
}

TileLoader::TileLoader(GeoStore& geoStore, StringTable& stringTable, const StyleProvider& styleProvider) :
    pimpl_(std::unique_ptr<TileLoader::TileLoaderImpl>(new TileLoader::TileLoaderImpl(geoStore, styleProvider, stringTable.getId(BuilderKeyName))))
{
}

TileLoader::~TileLoader()
{
}

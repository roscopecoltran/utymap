#include "Exceptions.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/TileBuilder.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "utils/CompatibilityUtils.hpp"
#include "utils/CoreUtils.hpp"

#include <stdexcept>
#include <sstream>
#include <unordered_map>

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const std::string BuilderKeyName = "builders";

class TileBuilder::TileBuilderImpl
{
private:

    typedef std::unordered_map<std::string, ElementBuilderFactory> BuilderFactoryMap;

class AggregateElemenVisitor : public ElementVisitor
{
public:
    AggregateElemenVisitor(const QuadKey& quadKey, 
                           const StyleProvider& styleProvider, 
                           StringTable& stringTable,
                           ElevationProvider& eleProvider,
                           const MeshCallback& meshFunc, 
                           const ElementCallback& elementFunc,
                           BuilderFactoryMap& builderFactoryMap,
                           std::uint32_t builderKeyId) :
        context_(quadKey, styleProvider, stringTable, eleProvider, meshFunc, elementFunc),
        builderFactoryMap_(builderFactoryMap),
        builderKeyId_(builderKeyId)
    {
    }

    void visitNode(const Node& node) { visitElement(node); }

    void visitWay(const Way& way) { visitElement(way); }

    void visitArea(const Area& area) { visitElement(area); }

    void visitRelation(const Relation& relation) { visitElement(relation); }

    void complete()
    {
        for (const auto& builder : builders_)
            builder.second->complete();
    }

private:

    // Calls appropriate visitor for given element
    void visitElement(const Element& element)
    {
        Style style = context_.styleProvider.forElement(element, context_.quadKey.levelOfDetail);
        std::stringstream ss(style.get(builderKeyId_));
        while (ss.good())
        {
            std::string name;
            getline(ss, name, ',');
            element.accept(getVisitor(name));
        }
    }

    ElementVisitor& getVisitor(const std::string& name)
    {
        auto visitorPair = builders_.find(name);
        if (visitorPair != builders_.end()) {
            return *visitorPair->second;
        }

        auto factory = builderFactoryMap_.find(name);
        if (factory == builderFactoryMap_.end()) {
            throw std::domain_error("Unknown element visitor");
        }

        auto visitor = factory->second(context_);
        builders_[name] = visitor;
        return *visitor;
    }

    const BuilderContext context_;
    BuilderFactoryMap& builderFactoryMap_;
    std::uint32_t builderKeyId_;
    std::unordered_map<std::string, std::shared_ptr<ElementBuilder>> builders_;
};

public:

    TileBuilderImpl(GeoStore& geoStore, StringTable& stringTable, ElevationProvider& eleProvider) :
        geoStore_(geoStore),
        stringTable_(stringTable),
        eleProvider_(eleProvider),
        builderKeyId_(stringTable.getId(BuilderKeyName)),
        builderFactory_()
    {
    }

    void registerElementVisitor(const std::string& name, ElementBuilderFactory factory)
    {
        builderFactory_[name] = factory;
    }

    void build(const QuadKey& quadKey, 
               const StyleProvider& styleProvider, 
               const MeshCallback& meshFunc, 
               const ElementCallback& elementFunc)
    {
        AggregateElemenVisitor elementVisitor(quadKey, styleProvider, stringTable_, 
            eleProvider_, meshFunc, elementFunc, builderFactory_, builderKeyId_);

        geoStore_.search(quadKey, styleProvider, elementVisitor);
        elementVisitor.complete();
    }

private:
    GeoStore& geoStore_;
    StringTable& stringTable_;
    ElevationProvider& eleProvider_;
    std::uint32_t builderKeyId_;
    BuilderFactoryMap builderFactory_;
};

void TileBuilder::registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
{
    pimpl_->registerElementVisitor(name, factory);
}

void TileBuilder::build(const QuadKey& quadKey, const StyleProvider& styleProvider, MeshCallback meshFunc, ElementCallback elementFunc)
{
    pimpl_->build(quadKey, styleProvider, meshFunc, elementFunc);
}

TileBuilder::TileBuilder(GeoStore& geoStore, StringTable& stringTable, ElevationProvider& eleProvider) :
    pimpl_(new TileBuilder::TileBuilderImpl(geoStore, stringTable, eleProvider))
{
}

TileBuilder::~TileBuilder()
{
}

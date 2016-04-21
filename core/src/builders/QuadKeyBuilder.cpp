#include "Exceptions.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/QuadKeyBuilder.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "utils/CoreUtils.hpp"

using namespace utymap;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::heightmap;
using namespace utymap::index;
using namespace utymap::mapcss;
using namespace utymap::meshing;

const std::string BuilderKeyName = "builders";

class QuadKeyBuilder::QuadKeyBuilderImpl
{
private:

    typedef std::unordered_map<std::string, ElementBuilderFactory> BuilderFactoryMap;

class AggregateElemenVisitor : public ElementVisitor
{
public:
    AggregateElemenVisitor(const QuadKey& quadKey,
                           const StyleProvider& styleProvider,
                           StringTable& stringTable,
                           const ElevationProvider& eleProvider,
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
        std::stringstream ss(*style.get(builderKeyId_)->value());
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

    QuadKeyBuilderImpl(GeoStore& geoStore, StringTable& stringTable) :
        geoStore_(geoStore),
        stringTable_(stringTable),
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
               const ElevationProvider& eleProvider,
               const MeshCallback& meshFunc,
               const ElementCallback& elementFunc)
    {
        AggregateElemenVisitor elementVisitor(quadKey, styleProvider, stringTable_,
            eleProvider, meshFunc, elementFunc, builderFactory_, builderKeyId_);

        geoStore_.search(quadKey, styleProvider, elementVisitor);
        elementVisitor.complete();
    }

private:
    GeoStore& geoStore_;
    StringTable& stringTable_;
    std::uint32_t builderKeyId_;
    BuilderFactoryMap builderFactory_;
};

void QuadKeyBuilder::registerElementBuilder(const std::string& name, ElementBuilderFactory factory)
{
    pimpl_->registerElementVisitor(name, factory);
}

void QuadKeyBuilder::build(const QuadKey& quadKey, const StyleProvider& styleProvider, const ElevationProvider& eleProvider, 
    MeshCallback meshFunc, ElementCallback elementFunc)
{
    pimpl_->build(quadKey, styleProvider, eleProvider, meshFunc, elementFunc);
}

QuadKeyBuilder::QuadKeyBuilder(GeoStore& geoStore, StringTable& stringTable) :
    pimpl_(new QuadKeyBuilder::QuadKeyBuilderImpl(geoStore, stringTable))
{
}

QuadKeyBuilder::~QuadKeyBuilder()
{
}

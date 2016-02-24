#include "Exceptions.hpp"
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
const std::string TerrainBuilderName = "terrain";

class TileBuilder::TileBuilderImpl
{
private:

    typedef std::unordered_map<std::string, ElementVisitorFactory> VisitorFactoryMap;

class AggregateElemenVisitor : public ElementVisitor
{
public:
    AggregateElemenVisitor(StringTable& stringTable,
                           const QuadKey& quadKey,
                           const StyleProvider& styleProvider,
                           ElevationProvider& eleProvider,
                           VisitorFactoryMap& visitorFactoryMap,
                           std::uint32_t visitorKeyId,
                           const MeshCallback& meshFunc,
                           const ElementCallback& elementFunc) :
        quadKey_(quadKey),
        styleProvider_(styleProvider),
        visitorFactoryMap_(visitorFactoryMap),
        visitorKeyId_(visitorKeyId),
        meshFunc_(meshFunc),
        elementFunc_(elementFunc)
    {
        visitorFactoryMap_[TerrainBuilderName] = [&](const utymap::QuadKey& quadKey,
                                                     const StyleProvider& styleProvider,
                                                     const MeshCallback& meshFunc,
                                                     const ElementCallback& elementFunc) {
            return std::shared_ptr<ElementVisitor>(new TerraBuilder(quadKey, styleProvider, stringTable, eleProvider, meshFunc));
        };
    }

    void visitNode(const Node& node) { visitElement(node); }

    void visitWay(const Way& way) { visitElement(way); }

    void visitArea(const Area& area) { visitElement(area); }

    void visitRelation(const Relation& relation) { visitElement(relation); }

    void complete()
    {
        auto terraBuilderPair = visitors_.find(TerrainBuilderName);
        if (terraBuilderPair != visitors_.end()) {
            dynamic_cast<TerraBuilder*>(terraBuilderPair->second.get())->complete();
        }
    }

private:

    // Calls appropriate visitor for given element
    void visitElement(const Element& element)
    {
        Style style = styleProvider_.forElement(element, quadKey_.levelOfDetail);
        std::stringstream ss(style.get(visitorKeyId_));
        while (ss.good())
        {
            std::string name;
            getline(ss, name, ',');
            element.accept(getVisitor(name));
        }
    }

    ElementVisitor& getVisitor(const std::string& name)
    {
        auto visitorPair = visitors_.find(name);
        if (visitorPair != visitors_.end()) {
            return *visitorPair->second;
        }

        auto factory = visitorFactoryMap_.find(name);
        if (factory == visitorFactoryMap_.end()) {
            throw std::domain_error("Unknown element visitor");
        }

        auto visitor = factory->second(quadKey_, styleProvider_, meshFunc_, elementFunc_);
        visitors_[name] = visitor;
        return *visitor;
    }

    utymap::QuadKey quadKey_;
    const StyleProvider& styleProvider_;
    const MeshCallback& meshFunc_;
    const ElementCallback& elementFunc_;
    VisitorFactoryMap& visitorFactoryMap_;
    std::uint32_t visitorKeyId_;
    std::unordered_map<std::string, std::shared_ptr<ElementVisitor>> visitors_;
};

public:

    TileBuilderImpl(GeoStore& geoStore, StringTable& stringTable, ElevationProvider& eleProvider) :
        geoStore_(geoStore),
        stringTable_(stringTable),
        eleProvider_(eleProvider),
        visitorKeyId_(stringTable.getId(BuilderKeyName)),
        visitorFactory_()
    {
    }

    void registerElementVisitor(const std::string& name, ElementVisitorFactory factory)
    {
        visitorFactory_[name] = factory;
    }

    void build(const QuadKey& quadKey, const StyleProvider& styleProvider, const MeshCallback& meshFunc, const ElementCallback& elementFunc)
    {
        AggregateElemenVisitor elementVisitor(stringTable_, quadKey, styleProvider, eleProvider_, visitorFactory_, visitorKeyId_, meshFunc, elementFunc);
        geoStore_.search(quadKey, styleProvider, elementVisitor);
        elementVisitor.complete();
    }

private:
    GeoStore& geoStore_;
    StringTable& stringTable_;
    ElevationProvider& eleProvider_;
    std::uint32_t visitorKeyId_;
    VisitorFactoryMap visitorFactory_;
};

void TileBuilder::registerElementVisitor(const std::string& name, ElementVisitorFactory factory)
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

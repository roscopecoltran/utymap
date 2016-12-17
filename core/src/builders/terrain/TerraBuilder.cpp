#include "BoundingBox.hpp"
#include "clipper/clipper.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "builders/terrain/SurfaceGenerator.hpp"
#include "builders/terrain/ExteriorGenerator.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"

#include "utils/GeometryUtils.hpp"

using namespace ClipperLib;
using namespace utymap::builders;
using namespace utymap::entities;
using namespace utymap::index;
using namespace utymap::heightmap;
using namespace utymap::mapcss;
using namespace utymap::utils;

namespace {
    const double Scale = 1E7;

    /// Converts coordinate to clipper's IntPoint.
    IntPoint toIntPoint(double x, double y)
    {
        return IntPoint(static_cast<cInt>(x * Scale), static_cast<cInt>(y * Scale));
    }

    /// Compares regions based on their area.
    struct LessThanByArea
    {
        bool operator()(const RegionPtr& lhs, const RegionPtr& rhs) const
        {
            return lhs->area < rhs->area;
        }
    };

    /// Visits relation and fills region.
    struct RelationVisitor : public ElementVisitor
    {
        const Relation& relation;
        ElementVisitor& builder;
        Region& region;

        RelationVisitor(ElementVisitor& b, const Relation& r, Region& reg) :
                relation(r), builder(b), region(reg) {}

        void visitNode(const utymap::entities::Node& n) override { n.accept(builder); }

        void visitWay(const utymap::entities::Way& w) override { w.accept(builder); }

        void visitArea(const utymap::entities::Area& a) override
        {
            Path path;
            path.reserve(a.coordinates.size());
            for (const utymap::GeoCoordinate& c : a.coordinates)
                path.push_back(toIntPoint(c.longitude, c.latitude));

            region.geometry.push_back(path);
            region.area += std::abs(utymap::utils::getArea(a.coordinates));
        }

        void visitRelation(const utymap::entities::Relation& r) override { r.accept(builder); }
    };
}

class TerraBuilder::TerraBuilderImpl final : public ElementBuilder
{
public:

    explicit TerraBuilderImpl(const BuilderContext& context) :
        ElementBuilder(context), 
        style_(context.styleProvider.forCanvas(context.quadKey.levelOfDetail)),
        generators_()
    {
        tileRect_.push_back(toIntPoint(context.boundingBox.minPoint.longitude, context.boundingBox.minPoint.latitude));
        tileRect_.push_back(toIntPoint(context.boundingBox.maxPoint.longitude, context.boundingBox.minPoint.latitude));
        tileRect_.push_back(toIntPoint(context.boundingBox.maxPoint.longitude, context.boundingBox.maxPoint.latitude));
        tileRect_.push_back(toIntPoint(context.boundingBox.minPoint.longitude, context.boundingBox.maxPoint.latitude));

        clipper_.AddPath(tileRect_, ptClip, true);

        // NOTE order is important due to propagation of region changes from top to bottom.
        generators_.push_back(utymap::utils::make_unique<ExteriorGenerator>(context, style_, tileRect_));
        generators_.push_back(utymap::utils::make_unique<SurfaceGenerator>(context, style_, tileRect_));
    }

    void visitNode(const utymap::entities::Node& node) override
    {
    }

    void visitWay(const utymap::entities::Way& way) override
    {
        Style style = context_.styleProvider.forElement(way, context_.quadKey.levelOfDetail);
        auto region = createRegion(style, way.coordinates);

        // make polygon from line by offsetting it using width specified
        double width = style.getValue(StyleConsts::WidthKey, 
            context_.boundingBox.maxPoint.latitude - context_.boundingBox.minPoint.latitude,
            context_.boundingBox.center()) * Scale;

        Paths solution;
        // NOTE: we should limit round shape precision due to performance reasons.
        offset_.ArcTolerance = width * 0.05;
        offset_.AddPaths(region->geometry, jtRound, etOpenRound);
        offset_.Execute(solution, width);
        offset_.Clear();

        clipper_.AddPaths(solution, ptSubject, true);
        clipper_.Execute(ctIntersection, solution);
        clipper_.removeSubject();
       
        region->geometry = solution;
        std::string type = region->isLayer()
            ? style.getString(StyleConsts::TerrainLayerKey)
            : "";

        addRegion(type, way, style, region);
    }

    void visitArea(const utymap::entities::Area& area) override
    {
        Style style = context_.styleProvider.forElement(area, context_.quadKey.levelOfDetail);
        auto region = createRegion(style, area.coordinates);
        std::string type = region->isLayer()
            ? style.getString(StyleConsts::TerrainLayerKey)
            : "";

        addRegion(type, area, style, region);
    }

    void visitRelation(const utymap::entities::Relation& rel) override
    {
        auto region = std::make_shared<Region>();
        RelationVisitor visitor(*this, rel, *region);

        for (const auto& element : rel.elements) {
            // if there are no tags, then this element is result of clipping
            if (element->tags.empty())
                element->tags = rel.tags;

            if (context_.styleProvider.hasStyle(*element, context_.quadKey.levelOfDetail))
                element->accept(visitor);
        }

        if (!region->geometry.empty()) {
            Style style = context_.styleProvider.forElement(rel, context_.quadKey.levelOfDetail);
            if (!style.has(context_.stringTable.getId(StyleConsts::TerrainLayerKey)))
                region->context = utymap::utils::make_unique<RegionContext>(RegionContext::create(context_, style, ""));

            std::string type = region->isLayer()
                ? style.getString(StyleConsts::TerrainLayerKey)
                : "";

            addRegion(type, rel, style, region);
        }
    }

    /// Builds tile mesh using created layers and registered terra generators.
    void complete() override
    {
        for (auto& layerPair : layers_) {
            if (!layerPair.first.empty()) {
                mergeRegions(layerPair.second, std::make_shared<RegionContext>(
                    RegionContext::create(context_, style_, layerPair.first + "-")));
            }
            // sort based on area
            std::sort(layerPair.second.begin(), layerPair.second.end(), LessThanByArea());
        }

        for (const auto& generator : generators_)
            generator->generateFrom(layers_);
    }

private:

    void addRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, std::shared_ptr<Region>& region)
    {
        for (const auto& generator : generators_) {
            layers_[type].push_back(region);
            generator->onNewRegion(type, element, style, region);
        }
    }

    /// Creates region from given geometry and style.
    std::shared_ptr<Region> createRegion(const Style& style, const std::vector<GeoCoordinate>& coordinates) const
    {
        auto region = std::make_shared<Region>();
        Path path;
        path.reserve(coordinates.size());
        for (const GeoCoordinate& c : coordinates)
            path.push_back(toIntPoint(c.longitude, c.latitude));

        region->geometry.push_back(path);

        if (!style.has(context_.stringTable.getId(StyleConsts::TerrainLayerKey)))
            region->context = std::make_shared<RegionContext>(RegionContext::create(context_, style, ""));

        int level = static_cast<int>(style.getValue(StyleConsts::LevelKey));
        region->origLevel = level;
        region->level = level;
        region->area = std::abs(utymap::utils::getArea(coordinates));

        return region;
    }

    /// Merges regions into one taking into account level difference.
    void mergeRegions(Layer& layer, std::shared_ptr<const RegionContext> regionContext) const
    {
        std::unordered_map<int, std::pair<std::shared_ptr<Region>, std::shared_ptr<Clipper>>> regionMap;
        for (const auto& current : layer) {
            auto mapPair = regionMap.find(current->level);
            if (mapPair == regionMap.end()) {
                auto clipperPair = std::make_pair(std::make_shared<Region>(), std::make_shared<Clipper>());
                mapPair = regionMap.insert(std::make_pair(current->level, clipperPair)).first;
                mapPair->second.first->level = current->level;
                mapPair->second.first->context = regionContext;
            }
            mapPair->second.first->area += current->area;
            mapPair->second.second->AddPaths(current->geometry, ptSubject, true);
        }

        layer.clear();
        for (auto& pair : regionMap) {
            Paths result;
            pair.second.second->Execute(ctUnion, result, pftNonZero, pftNonZero);
            pair.second.first->geometry = std::move(result);
            layer.push_back(pair.second.first);
        }
    }

    const Style style_;
    ClipperEx clipper_;
    ClipperOffset offset_;
    std::vector<std::unique_ptr<TerraGenerator>> generators_;
    Layers layers_;
    Path tileRect_;
};

void TerraBuilder::visitNode(const utymap::entities::Node& node) { pimpl_->visitNode(node); }

void TerraBuilder::visitWay(const utymap::entities::Way& way) { pimpl_->visitWay(way); }

void TerraBuilder::visitArea(const utymap::entities::Area& area) { pimpl_->visitArea(area); }

void TerraBuilder::visitRelation(const utymap::entities::Relation& relation) { pimpl_->visitRelation(relation); }

void TerraBuilder::complete() { pimpl_->complete(); }

TerraBuilder::~TerraBuilder() { }

TerraBuilder::TerraBuilder(const BuilderContext& context) :
    ElementBuilder(context), pimpl_(utymap::utils::make_unique<TerraBuilderImpl>(context))
{
}

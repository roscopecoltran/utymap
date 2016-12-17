#include "builders/terrain/ExteriorGenerator.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "math/Vector2.hpp"
#include "utils/GeometryUtils.hpp"

#include <climits>
#include <map>
#include <unordered_set>
#include <unordered_map>

using namespace ClipperLib;
using namespace utymap;
using namespace utymap::builders;
using namespace utymap::mapcss;
using namespace utymap::math;

namespace {
const double Scale = 1E7;

const std::string TerrainMeshName = "terrain_exterior";

const std::string InclineKey = "incline";
const std::string InclineUpValue = "up";
const std::string InclineDownValue = "down";

double interpolate(double a, double b, double r)
{
    return a * (1.0 - r) + b * r;
}

struct HashFunc
{
    size_t operator()(const Vector2& v) const
    {
        size_t h1 = std::hash<double>()(v.x);
        size_t h2 = std::hash<double>()(v.y);
        return h1 ^ h2;
    }
};

struct EqualsFunc
{
    bool operator()(const Vector2& lhs, const Vector2& rhs) const
    {
        return lhs == rhs;
    }
};

struct InclineType final
{
    const static InclineType None;
    const static InclineType Up;
    const static InclineType Down;

    static InclineType create(const std::string& value)
    {
        if (value == InclineUpValue) return Up;
        if (value == InclineDownValue) return Down;
        // TODO parse actual value
        return None;
    }

    int getValue() const { return value_; }

private:
    explicit InclineType(int value) : value_(value)
    {
    }

    int value_;
};

bool operator==(const InclineType& lhs, const InclineType& rhs) { return lhs.getValue() == rhs.getValue(); }

bool operator!=(const InclineType& lhs, const InclineType& rhs) { return lhs.getValue() != rhs.getValue(); }

const InclineType InclineType::None = InclineType(0);
const InclineType InclineType::Up = InclineType(std::numeric_limits<int>::max());
const InclineType InclineType::Down = InclineType(std::numeric_limits<int>::min());

/// Stores information for building slope region.
struct SlopeSegment final
{
    const Vector2 tail;
    const double width;

    // NOTE for debug only.
    // TODO remove
    std::size_t elementId;
    
    SlopeSegment(const Vector2& tail, double width, std::size_t elementId) :
        tail(tail),  width(width), elementId(elementId)
    {
    }
};

/// Encapsulates slope region used to calculate slope ration for given point.
class SlopeRegion final
{
public:
    SlopeRegion(const Vector2& start, const Vector2& end, double width, std::size_t elementId) :
        elementId(elementId), start_(start), centerLine_(end - start),
        geometry(utymap::utils::getOffsetLine(start, end + centerLine_.normalized() * width, width)),
        lengthSquare_()
    {
        double distance = Vector2::distance(start, end);
        lengthSquare_ = distance * distance;
    }

    /// Returns true if the point is inside geometry.
    bool contains(const Vector2& p) const
    {
        return utymap::utils::isPointInPolygon(p, geometry);
    }

    /// Calculate scalar projection divided to length to get slope ratio in range [0, 1]
    double calculateSlope(const Vector2& v) const
    {
        return utymap::utils::clamp((v - start_).dot(centerLine_) / lengthSquare_, 0, 1);
    }
    // NOTE for debug only.
    // TODO remove
    std::size_t elementId;

private:
    const Vector2 start_;
    const Vector2 centerLine_;
    const std::vector<Vector2> geometry;
    double lengthSquare_;
};

}

class ExteriorGenerator::ExteriorGeneratorImpl : public utymap::entities::ElementVisitor
{
    /// Defines a type for storing all possible slope segments.
    typedef std::unordered_map<Vector2, std::vector<SlopeSegment>, HashFunc, EqualsFunc> ExitMap;

public:
    ExteriorGeneratorImpl(const BuilderContext& builderContext) :
        builderContext_(builderContext),  levelInfoMap_(), slopeRegionMap_(), 
        inclineKey_(builderContext.stringTable.getId(InclineKey)),
        region_(nullptr), style_(nullptr), inclineType_(InclineType::None)
    {
    }

    void visitNode(const utymap::entities::Node&) override 
    { 
        // TODO Some exits can be added as node?
    }

    void visitWay(const utymap::entities::Way& way) override
    {
        const auto& c1 = way.coordinates[0];
        const auto& c2 = way.coordinates[way.coordinates.size() - 1];

        auto v1 = Vector2(c1.longitude, c1.latitude);
        auto v2 = Vector2(c2.longitude, c2.latitude);

        double width = style_->getValue(StyleConsts::WidthKey,
            builderContext_.boundingBox.maxPoint.latitude - builderContext_.boundingBox.minPoint.latitude,
            builderContext_.boundingBox.center());

        auto levelInfoPair = levelInfoMap_.find(region_->level);
        if (levelInfoPair == levelInfoMap_.end()) {
            levelInfoMap_.insert(std::make_pair(region_->level, utymap::utils::make_unique<ExitMap>()));
            levelInfoPair = levelInfoMap_.find(region_->level);
        }

        if (inclineType_ == InclineType::None) {
            // store as possible slope segment.
            (*levelInfoPair->second)[v1].push_back(SlopeSegment(v2, width, way.id));
            (*levelInfoPair->second)[v2].push_back(SlopeSegment(v1, width, way.id));
        }
        else {
            // promote down and add directly as slope region
            region_->level -= 1;
            if (inclineType_ == InclineType::Up)
                slopeRegionMap_[region_->level].push_back(SlopeRegion(v1, v2, width, way.id));
            else
                slopeRegionMap_[region_->level].push_back(SlopeRegion(v2, v1, width, way.id));
        }
    }

    void visitArea(const utymap::entities::Area& area) override 
    {
        // TODO area as a slope region?
    }

    void visitRelation(const utymap::entities::Relation& relation) override
    {
        for (const auto& element : relation.elements)
            element->accept(*this);
    }

    void setElementContext(const utymap::entities::Element& element, const Style& style, const std::shared_ptr<Region>& region)
    {
        inclineType_ = InclineType::create(style.getString(inclineKey_));
        style_ = &style;
        region_ = region;
    }

    void build()
    {
        // process tunnels
        for (auto curr = levelInfoMap_.begin(); curr != levelInfoMap_.end() && curr->first < 0; ++curr) {
            auto next = std::next(curr, 1);
            if (next == levelInfoMap_.end()) break;
            if (next->second->empty()) continue;

            // try to find exists on level above and create slope regions.
            for (const auto& slopePair : *curr->second) {
                // not an exit
                if (next->second->find(slopePair.first) == next->second->end()) continue;
                // an exit.
                for (const auto& segment : slopePair.second)
                    slopeRegionMap_[curr->first].push_back(SlopeRegion(slopePair.first, segment.tail, segment.width, segment.elementId));
            }
        }

        //// TODO process bridges
        //for (auto curr = levelInfoMap_.rbegin(); curr != levelInfoMap_.rend() && curr->first > 0; ++curr) {

        //}
    }

    double getHeight(int level, const GeoCoordinate& coordinate) const
    {
        const double deepHeight = 4;

        double startHeight = level * deepHeight;
        double endHeight = (level + 1) * deepHeight;

        auto regionPair = slopeRegionMap_.find(level);
        if (regionPair != slopeRegionMap_.end()) {
            Vector2 p(coordinate.longitude, coordinate.latitude);
            for (const auto& region : regionPair->second) {
                if (region.contains(p)) {
                    double slope = region.calculateSlope(p);
                    return interpolate(startHeight, endHeight, slope);
                }
            }
        }

        return startHeight;
    }

private:
    const BuilderContext& builderContext_;

    /// Stores information about level's exits.
    std::map<int, std::unique_ptr<ExitMap>> levelInfoMap_;

    /// Stores slope regions.
    std::unordered_map<int, std::vector<SlopeRegion>> slopeRegionMap_;

    /// Mapcss cached string ids.
    std::uint32_t inclineKey_;

    /// Current element style.
    std::shared_ptr<Region> region_;

    /// Current element style.
    const Style* style_;

    /// Current element region incline type.
    InclineType inclineType_;
};

ExteriorGenerator::ExteriorGenerator(const BuilderContext& context, const Style& style, const Path& tileRect) :
    TerraGenerator(context, style, tileRect, TerrainMeshName), p_impl(utymap::utils::make_unique<ExteriorGeneratorImpl>(context))
{
}

void ExteriorGenerator::onNewRegion(const std::string& type, const utymap::entities::Element& element, const Style& style, const std::shared_ptr<Region>& region)
{
    p_impl->setElementContext(element, style, region);
    element.accept(*p_impl);
}

void ExteriorGenerator::generateFrom(Layers& layers)
{
    p_impl->build();

    for (const auto& layerPair : layers) {
        for (const auto& region : layerPair.second) {
            // NOTE we don't have any slope regions on surface.
            if (region->level != 0) {
                TerraGenerator::addGeometry(region->level, region->geometry, *region->context, [](const Path& path) {});
            }
        }
    }

    context_.meshCallback(mesh_);
}

ExteriorGenerator::~ExteriorGenerator()
{
}

void ExteriorGenerator::addGeometry(int level, utymap::math::Polygon& polygon, const RegionContext& regionContext)
{
    context_.meshBuilder.addPolygon(mesh_, polygon, regionContext.geometryOptions, regionContext.appearanceOptions,
        [&](const GeoCoordinate& coordinate) {
            return p_impl->getHeight(level, coordinate);
    });
    context_.meshBuilder.writeTextureMappingInfo(mesh_, regionContext.appearanceOptions);
}

#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "index/ElementStore.hpp"
#include "index/ElementGeometryClipper.hpp"

using namespace utymap;
using namespace utymap::entities;

using namespace utymap::mapcss;

namespace {
    // Max precision for Lat/Lon
    const double Scale = 1E7;

    using PointLocation = utymap::index::ElementGeometryClipper::PointLocation;

    inline PointLocation checkWay(const BoundingBox& bbox, const Way& way, ClipperLib::Path& wayShape) {
        wayShape.reserve(way.coordinates.size());
        bool allInside = true;
        bool allOutside = true;
        for (const GeoCoordinate& coord : way.coordinates) {
            bool contains = bbox.contains(coord);
            allInside &= contains;
            allOutside &= !contains;

            auto x = static_cast<ClipperLib::cInt>(coord.longitude * Scale);
            auto y = static_cast<ClipperLib::cInt>(coord.latitude * Scale);
            wayShape.push_back(ClipperLib::IntPoint(x, y));
        }

        return allInside ? PointLocation::AllInside :
            (allOutside ? PointLocation::AllOutside : PointLocation::Mixed);
    }

    inline PointLocation checkArea(const BoundingBox& bbox, const Area& area, ClipperLib::Path& areaShape) {
        bool allInside = true;
        auto areaBbox = BoundingBox();

        areaShape.reserve(area.coordinates.size());
        for (const GeoCoordinate& coord : area.coordinates) {
            bool contains = bbox.contains(coord);
            areaBbox.expand(coord);
            allInside &= contains;

            auto x = static_cast<ClipperLib::cInt>(coord.longitude * Scale);
            auto y = static_cast<ClipperLib::cInt>(coord.latitude * Scale);
            areaShape.push_back(ClipperLib::IntPoint(x, y));
        }

        return allInside ? PointLocation::AllInside :
            (bbox.intersects(areaBbox) ? PointLocation::Mixed : PointLocation::AllOutside);
    }

    template<typename T>
    inline void setCoordinates(T& t, const ClipperLib::Path& path) {
        t.coordinates.reserve(path.size());
        for (const auto& c : path) {
            t.coordinates.push_back(GeoCoordinate(c.Y / Scale, c.X / Scale));
        }
    }

    template<typename T>
    inline void setData(T& t, const utymap::entities::Element& element, const ClipperLib::Path& path) {
        t.id = element.id;
        t.tags = element.tags;
        setCoordinates<T>(t, path);
    }

    ClipperLib::Path createPathFromBoundingBox(const BoundingBox& quadKeyBbox)
    {
        double xMin = quadKeyBbox.minPoint.longitude, yMin = quadKeyBbox.minPoint.latitude,
            xMax = quadKeyBbox.maxPoint.longitude, yMax = quadKeyBbox.maxPoint.latitude;
        ClipperLib::Path rect;
        rect.push_back(ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(xMin*Scale),
            static_cast<ClipperLib::cInt>(yMin*Scale)));

        rect.push_back(ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(xMax*Scale),
            static_cast<ClipperLib::cInt>(yMin*Scale)));

        rect.push_back(ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(xMax*Scale),
            static_cast<ClipperLib::cInt>(yMax*Scale)));

        rect.push_back(ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(xMin*Scale),
            static_cast<ClipperLib::cInt>(yMax*Scale)));
        return std::move(rect);
    }

    std::shared_ptr<Element> processWay(ClipperLib::ClipperEx& clipper, const BoundingBox& bbox, const Way& way)
    {
        ClipperLib::Path wayShape;
        PointLocation pointLocation = checkWay(bbox, way, wayShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            return std::make_shared<Way>(way);
        }

        // 2. all geometry outside : way should be skipped
        if (pointLocation == PointLocation::AllOutside) {
            return nullptr;
        }

        ClipperLib::PolyTree solution;
        clipper.AddPath(wayShape, ClipperLib::ptSubject, false);
        clipper.Execute(ClipperLib::ctIntersection, solution);
        clipper.removeSubject();
        
        std::size_t count = static_cast<std::size_t>(solution.Total());

        // 3. way intersects border only once: store a copy with clipped geometry
        if (count == 1) {
            auto clippedWay = std::make_shared<Way>();
            setData(*clippedWay, way, solution.GetFirst()->Contour);
            return clippedWay;
        }
        // 4. in this case, result should be stored as relation (collection of ways)
        else if (count > 1) {
            auto relation = std::make_shared<Relation>();
            relation->id = way.id;
            relation->tags = way.tags;
            relation->elements.reserve(count);
            ClipperLib::PolyNode* polyNode = solution.GetFirst();
            while (polyNode) {
                auto clippedWay = std::make_shared<Way>();
                clippedWay->id = way.id;
                setCoordinates(*clippedWay, polyNode->Contour);
                relation->elements.push_back(clippedWay);
                polyNode = polyNode->GetNext();
            }
            return relation;
        }

        // no intersection
        return nullptr;
    }

    std::shared_ptr<Element> processArea(ClipperLib::ClipperEx& clipper, const BoundingBox& bbox, const Area& area)
    {
        ClipperLib::Path areaShape;
        PointLocation pointLocation = checkArea(bbox, area, areaShape);
        // 1. all geometry inside current quadkey: no need to truncate.
        if (pointLocation == PointLocation::AllInside) {
            return std::make_shared<Area>(area);
        }

        // 2. all geometry outside: use geometry of quadkey
        if (pointLocation == PointLocation::AllOutside) {
            return nullptr;
        }

        ClipperLib::Paths solution;
        clipper.AddPath(areaShape, ClipperLib::ptSubject, true);
        clipper.Execute(ClipperLib::ctIntersection, solution);
        clipper.removeSubject();

        // 3. way intersects border only once: store a copy with clipped geometry
        if (solution.size() == 1) {
            auto clippedArea = std::make_shared<Area>();
            setData(*clippedArea, area, solution[0]);
            return clippedArea;
        }
        // 4. in this case, result should be stored as relation (collection of areas)
        else if (solution.size() > 0) {
            auto relation = std::make_shared<Relation>();
            relation->id = area.id;
            relation->tags = area.tags;
            relation->elements.reserve(solution.size());
            for (auto it = solution.begin(); it != solution.end(); ++it) {
                auto clippedArea = std::make_shared<Area>();
                clippedArea->id = area.id;
                setCoordinates(*clippedArea, *it);
                relation->elements.push_back(clippedArea);
            }
            return relation;
        }

        return nullptr;
    }

    std::shared_ptr<Element> processRelation(ClipperLib::ClipperEx& clipper, const BoundingBox& bbox, const Relation& relation);
 
    // Visits relation and collects clipped elements
    struct RelationVisitor : public ElementVisitor
    {
        RelationVisitor(ClipperLib::ClipperEx& clipper, const BoundingBox& quadKeyBbox) :
            clipper_(clipper), bbox_(quadKeyBbox), relation(nullptr)
        {
        }

        void visitNode(const Node& node)
        {
            if (bbox_.contains(node.coordinate)) {
                ensureRelation();
                relation->elements.push_back(std::make_shared<Node>(node));
            }
        }

        void visitWay(const Way& way)
        {
            addElement(processWay(clipper_, bbox_, way), way);
        }

        void visitArea(const Area& area)
        {
            addElement(processArea(clipper_, bbox_, area),area);
        }

        void visitRelation(const Relation& relation)
        {
            addElement(processRelation(clipper_, bbox_, relation), relation);
        }

        std::shared_ptr<Relation> relation;

    private:

        inline void ensureRelation()
        {
            if (relation == nullptr)
                relation = std::make_shared<Relation>();
        }

        void addElement(std::shared_ptr<Element> element, const Element& original)
        {
            if (element == nullptr)
                return;

            ensureRelation();

            element->id = original.id;
            element->tags = original.tags;

            relation->elements.push_back(element);
        }

        ClipperLib::ClipperEx& clipper_;
        const BoundingBox& bbox_;
    };

    std::shared_ptr<Element> processRelation(ClipperLib::ClipperEx& clipper, const BoundingBox& bbox, const Relation& relation)
    {
        RelationVisitor visitor(clipper, bbox);

        for (const auto& element : relation.elements) 
            element->accept(visitor);

        if (visitor.relation == nullptr)
            return nullptr;
        
        std::shared_ptr<Element> element = visitor.relation->elements.size() == 1
            ? visitor.relation->elements.at(0)
            : visitor.relation;

        element->id = relation.id;
        element->tags = relation.tags;

        return element;
    }
}

namespace utymap { namespace index {

ElementGeometryClipper::ElementGeometryClipper(Callback callback) :
    callback_(callback), quadKey_(), quadKeyBbox_(), clipper_()
{
}

void ElementGeometryClipper::clipAndCall(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
{
    quadKey_ = quadKey;
    quadKeyBbox_ = quadKeyBbox;
    clipper_.Clear();
    clipper_.AddPath(createPathFromBoundingBox(quadKeyBbox_), ClipperLib::ptClip, true);
    element.accept(*this);
}

void ElementGeometryClipper:: visitNode(const Node& node)
{
    if (quadKeyBbox_.contains(node.coordinate))
        callback_(node, quadKey_);
}

void ElementGeometryClipper::visitWay(const Way& way)
{
    auto element = processWay(clipper_, quadKeyBbox_, way);
    if (element != nullptr)
        callback_(*element, quadKey_);
}

void ElementGeometryClipper::visitArea(const Area& area)
{
    auto element = processArea(clipper_, quadKeyBbox_, area);
    if (element != nullptr)
        callback_(*element, quadKey_);
}

void ElementGeometryClipper::visitRelation(const Relation& relation)
{
    auto element = processRelation(clipper_, quadKeyBbox_, relation);
    if (element != nullptr)
        callback_(*element, quadKey_);
}

}}

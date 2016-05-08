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
using namespace utymap::formats;

using namespace utymap::mapcss;

namespace {
    using PointLocation = utymap::index::ElementGeometryClipper::PointLocation;

    template<typename T>
    inline PointLocation setPath(const BoundingBox& bbox, const T& t, ClipperLib::Path& shape) {
        shape.reserve(t.coordinates.size());
        bool allInside = true;
        bool allOutside = true;
        for (const GeoCoordinate& coord : t.coordinates) {
            bool contains = bbox.contains(coord);
            allInside &= contains;
            allOutside &= !contains;
            shape.push_back(ClipperLib::IntPoint(coord.longitude * utymap::index::ElementGeometryClipper::Scale,
                                                 coord.latitude * utymap::index::ElementGeometryClipper::Scale));
        }

        return allInside ? PointLocation::AllInside :
               (allOutside ? PointLocation::AllOutside : PointLocation::Mixed);
    }
}

namespace utymap { namespace index {

const double ElementGeometryClipper::Scale = 1E7;

void ElementGeometryClipper::clipAndCall(const Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox)
{
    quadKeyPtr_ = &quadKey;
    quadKeyBboxPtr_ = &quadKeyBbox;
    element.accept(*this);
}

void ElementGeometryClipper:: visitNode(const Node& node)
{
    // here, node should be always in tile
    callback_(node, *quadKeyPtr_);
}

void ElementGeometryClipper::visitWay(const Way& way)
{
    ClipperLib::Path wayShape;
    PointLocation pointLocation = setPath(*quadKeyBboxPtr_, way, wayShape);
    // 1. all geometry inside current quadkey: no need to truncate.
    if (pointLocation == PointLocation::AllInside) {
        callback_(way, *quadKeyPtr_);
        return;
    }

    // 2. all geometry outside : way should be skipped
    if (pointLocation == PointLocation::AllOutside) {
        return;
    }

    ClipperLib::PolyTree solution;
    clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
    clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
    clipper_.Execute(ClipperLib::ctIntersection, solution);
    clipper_.Clear();
    int count = solution.Total();

    // 3. way intersects border only once: store a copy with clipped geometry
    if (count == 1) {
        Way clippedWay;
        setData(clippedWay, way, solution.GetFirst()->Contour);
        callback_(clippedWay, *quadKeyPtr_);
    }
        // 4. in this case, result should be stored as relation (collection of ways)
    else {
        Relation relation;
        relation.id = way.id;
        relation.tags = way.tags;
        relation.elements.reserve(count);
        ClipperLib::PolyNode* polyNode = solution.GetFirst();
        while (polyNode) {
            std::shared_ptr<Way> clippedWay(new Way());
            clippedWay->id = way.id;
            setCoordinates(*clippedWay, polyNode->Contour);
            relation.elements.push_back(clippedWay);
            polyNode = polyNode->GetNext();
        }
        callback_(relation, *quadKeyPtr_);
    }
}

void ElementGeometryClipper::visitArea(const Area& area)
{
    ClipperLib::Path areaShape;
    PointLocation pointLocation = setPath(*quadKeyBboxPtr_, area, areaShape);
    // 1. all geometry inside current quadkey: no need to truncate.
    if (pointLocation == PointLocation::AllInside) {
        callback_(area, *quadKeyPtr_);
        return;
    }

    // 2. all geometry outside : pass empty
    if (pointLocation == PointLocation::AllOutside) {
        Area emptyArea;
        emptyArea.id = area.id;
        emptyArea.tags = area.tags;
        callback_(emptyArea, *quadKeyPtr_);
        return;
    }

    ClipperLib::Paths solution;
    clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
    clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
    clipper_.Execute(ClipperLib::ctIntersection, solution);
    clipper_.Clear();

    // 3. way intersects border only once: store a copy with clipped geometry
    if (solution.size() == 1) {
        Area clippedArea;
        setData(clippedArea, area, solution[0]);
        callback_(clippedArea, *quadKeyPtr_);
    }
        // 4. in this case, result should be stored as relation (collection of areas)
    else {
        Relation relation;
        relation.id = area.id;
        relation.tags = area.tags;
        relation.elements.reserve(solution.size());
        for (auto it = solution.begin(); it != solution.end(); ++it) {
            std::shared_ptr<Area> clippedArea(new Area());
            clippedArea->id = area.id;
            setCoordinates(*clippedArea, *it);
            relation.elements.push_back(clippedArea);
        }
        callback_(relation, *quadKeyPtr_);
    }
}

void ElementGeometryClipper::visitRelation(const Relation& relation)
{
    struct RelationVisitor : public ElementVisitor
    {
        Relation data;

        RelationVisitor(const BoundingBox& quadKeyBbox, ClipperLib::Clipper& clipper) :
                bbox_(quadKeyBbox), clipper_(clipper) { }

        void visitNode(const Node& node)
        {
            if (bbox_.contains(node.coordinate)) {
                data.elements.push_back(std::shared_ptr<Node>(new Node(node)));
            }
        }

        void visitWay(const Way& way)
        {
            ClipperLib::Path wayShape;
            setPath(bbox_, way, wayShape);
            clipper_.AddPath(wayShape, ClipperLib::ptSubject, false);
        }

        void visitArea(const Area& area)
        {
            ClipperLib::Path areaShape;
            setPath(bbox_, area, areaShape);
            clipper_.AddPath(areaShape, ClipperLib::ptSubject, true);
        }

        void visitRelation(const Relation& relation)
        {
            for (const auto& element : relation.elements) {
                element->accept(*this);
            }
        }

    private:
        const BoundingBox& bbox_;
        ClipperLib::Clipper& clipper_;

    } visitor(*quadKeyBboxPtr_, clipper_);

    relation.accept(visitor);

    // Process results
    ClipperLib::PolyTree solution;
    clipper_.AddPath(createPathFromBoundingBox(), ClipperLib::ptClip, true);
    clipper_.Execute(ClipperLib::ctIntersection, solution);
    clipper_.Clear();

    int count = solution.Total();
    // Do not store one result as relation
    if (count == 1) {
        ClipperLib::PolyNode* node = solution.GetFirst();
        if (node->IsOpen()) {
            Way way;
            setData(way, relation, node->Contour);
            callback_(way, *quadKeyPtr_);
        }
        else {
            if (!node->IsHole()) {
                Area area;
                setData(area, relation, node->Contour);
                callback_(area, *quadKeyPtr_);
            }
        }
    }
    else if (count > 1) {
        Relation newRelation;
        newRelation.id = relation.id;
        newRelation.tags = relation.tags;
        newRelation.elements.reserve(count);

        ClipperLib::PolyNode* polyNode = solution.GetFirst();
        while (polyNode) {
            if (polyNode->IsOpen()) {
                std::shared_ptr<Way> way(new Way());
                setCoordinates(*way, polyNode->Contour);
                newRelation.elements.push_back(way);
            }
            else {
                std::shared_ptr<Area> area(new Area());
                setCoordinates(*area, polyNode->Contour);
                newRelation.elements.push_back(area);
            }
            polyNode = polyNode->GetNext();
        }
        callback_(newRelation, *quadKeyPtr_);
    }
}

ClipperLib::Path ElementGeometryClipper::createPathFromBoundingBox()
{
    double xMin = quadKeyBboxPtr_->minPoint.longitude, yMin = quadKeyBboxPtr_->minPoint.latitude,
            xMax = quadKeyBboxPtr_->maxPoint.longitude, yMax = quadKeyBboxPtr_->maxPoint.latitude;
    ClipperLib::Path rect;
    rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMin*Scale));
    rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMin*Scale));
    rect.push_back(ClipperLib::IntPoint(xMax*Scale, yMax*Scale));
    rect.push_back(ClipperLib::IntPoint(xMin*Scale, yMax*Scale));
    return std::move(rect);
}

}}

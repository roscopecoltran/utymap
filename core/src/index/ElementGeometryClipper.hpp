#ifndef INDEX_ELEMENTGEOMETRYVISITOR_HPP_DEFINED
#define INDEX_ELEMENTGEOMETRYVISITOR_HPP_DEFINED

#include "clipper/clipper.hpp"
#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "entities/Node.hpp"
#include "entities/Way.hpp"
#include "entities/Area.hpp"
#include "entities/Relation.hpp"
#include "entities/ElementVisitor.hpp"
#include "index/ElementStore.hpp"

#include <functional>

namespace utymap { namespace index {

// Modifies geometry of element by bounding box clipping.
class ElementGeometryClipper : private utymap::entities::ElementVisitor
{
public:
    // Max precision for Lat/Lon
    static const double Scale;
    // Defines callback
    typedef std::function<void(const utymap::entities::Element& element, const utymap::QuadKey& quadKey)> Callback;
    // Defines polygon points location relative to current quadkey.
    enum PointLocation { AllInside, AllOutside, Mixed };

    ElementGeometryClipper(Callback callback) :
            callback_(callback),
            quadKeyPtr_(nullptr),
            quadKeyBboxPtr_(nullptr)
    {
    }

    void clipAndCall(const utymap::entities::Element& element, const QuadKey& quadKey, const BoundingBox& quadKeyBbox);

private:

    void visitNode(const utymap::entities::Node& node);

    void visitWay(const utymap::entities::Way& way);

    void visitArea(const utymap::entities::Area& area);

    void visitRelation(const utymap::entities::Relation& relation);

    ClipperLib::Path createPathFromBoundingBox();

    template<typename T>
    inline void setData(T& t, const utymap::entities::Element& element, const ClipperLib::Path& path) {
        t.id = element.id;
        t.tags = element.tags;
        setCoordinates<T>(t, path);
    }

    template<typename T>
    inline void setCoordinates(T& t, const ClipperLib::Path& path) {
        t.coordinates.reserve(path.size());
        for (const auto& c : path) {
            t.coordinates.push_back(GeoCoordinate(c.Y / Scale, c.X / Scale));
        }
    }

    Callback callback_;
    const QuadKey* quadKeyPtr_;
    const BoundingBox* quadKeyBboxPtr_;
    ClipperLib::Clipper clipper_;
};

}}

#endif //INDEX_ELEMENTGEOMETRYVISITOR_HPP_DEFINED

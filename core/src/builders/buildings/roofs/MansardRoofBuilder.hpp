#ifndef BUILDERS_BUILDINGS_ROOFS_MANSARDROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_MANSARDROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "clipper/clipper.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"
#include "meshing/Polygon.hpp"

#include <algorithm>
#include <limits>
#include <vector>

namespace utymap { namespace builders {

// Builds mansard roof.
class MansardRoofBuilder final : public FlatRoofBuilder
{
    const double Scale = 1E7;

public:
    MansardRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                       utymap::builders::MeshContext& meshContext)
        : FlatRoofBuilder(builderContext, meshContext)
    {
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        ClipperLib::ClipperOffset offset;
        ClipperLib::Path path;
        path.reserve(polygon.points.size() / 2);

        auto lastPointIndex = polygon.points.size() - 2;
        double min = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < polygon.points.size(); i += 2) {
            auto nextIndex = i == lastPointIndex ? 0 : i + 2;

            utymap::meshing::Vector2 v1(polygon.points[i], polygon.points[i + 1]);
            utymap::meshing::Vector2 v2(polygon.points[nextIndex], polygon.points[nextIndex + 1]);

            min = std::min(min, utymap::meshing::Vector2::distance(v1, v2));

            path.push_back(ClipperLib::IntPoint(static_cast<ClipperLib::cInt>(v1.x * Scale), 
                                                static_cast<ClipperLib::cInt>(v1.y * Scale)));
        }

        offset.AddPath(path, ClipperLib::JoinType::jtMiter, ClipperLib::EndType::etClosedPolygon);

        ClipperLib::Paths solution;
        // NOTE: use minimal side value as reference for offsetting.
        offset.Execute(solution, -(min / 10) * Scale);

        // NOTE: this is unexpected result for algorithm below, fallback to flat roof.
        if (solution.size() != 1 || solution[0].size() != path.size()) {
            return FlatRoofBuilder::build(polygon);
        }

        buildMansardShape(polygon, solution[0], findFirstIndex(solution[0][0], polygon));
    }

private:

    void buildMansardShape(const utymap::meshing::Polygon& polygon, ClipperLib::Path& offsetShape, std::size_t index) const
    {
        if (!ClipperLib::Orientation(offsetShape))
            std::reverse(offsetShape.begin(), offsetShape.end());

        // build top
        utymap::meshing::Polygon topShape(offsetShape.size(), 0);
        std::vector<utymap::meshing::Vector2> topShapeVertices;
        topShapeVertices.reserve(offsetShape.size());
        for (const auto& p : offsetShape) {
            topShapeVertices.push_back(utymap::meshing::Vector2(p.X / Scale, p.Y/ Scale));
        }
        topShape.addContour(topShapeVertices);

        auto topOptions = utymap::meshing::MeshBuilder::Options(0, 0, colorNoiseFreq_,
                                                                height_, meshContext_.gradient, minHeight_);
        builderContext_.meshBuilder.addPolygon(meshContext_.mesh, topShape, topOptions);

        // build sides
        auto sideOptions = utymap::meshing::MeshBuilder::Options(0, 0, colorNoiseFreq_,
                                                                 0, meshContext_.gradient, 0);
        double topHeight = minHeight_ + height_;
        auto size = polygon.points.size();
        for (std::size_t i = 0; i < size; i += 2) {
            auto topIndex = i;
            auto bottomIndex = (index + i) % size;
            auto nextTopIndex = (i + 2)  % size;
            auto nextBottomIndex = (index + i + 2) % size;

            auto v0 = utymap::meshing::Vector3(polygon.points[bottomIndex], minHeight_, polygon.points[bottomIndex + 1]);
            auto v1 = utymap::meshing::Vector3(polygon.points[nextBottomIndex], minHeight_, polygon.points[nextBottomIndex + 1]);
            auto v2 = utymap::meshing::Vector3(topShape.points[nextTopIndex], topHeight, topShape.points[nextTopIndex + 1]);
            auto v3 = utymap::meshing::Vector3(topShape.points[topIndex], topHeight, topShape.points[topIndex + 1]);

            builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v2, v0, v3, sideOptions, false);
            builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v2, v1, sideOptions, false);
        }
    }

    std::size_t findFirstIndex(const ClipperLib::IntPoint& p, const utymap::meshing::Polygon& polygon) const
    {
        utymap::meshing::Vector2 point(p.X / Scale, p.Y / Scale);

        std::size_t index = 0, size = polygon.points.size() / 2;
        double minDistance = std::numeric_limits<double>::max();

        for (std::size_t i = 0; i < size; i+=2) {
            double distance = utymap::meshing::Vector2::distance(point, 
                utymap::meshing::Vector2(polygon.points[i], polygon.points[i + 1]));
            if (distance < minDistance) {
                minDistance = distance;
                index = i;
            }
        }

        return index;
    }
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_MANSARDROOFBUILDER_HPP_DEFINED

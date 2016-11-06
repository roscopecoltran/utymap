#ifndef BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MathUtils.hpp"
#include "utils/MeshUtils.hpp"

#include <cmath>

namespace utymap { namespace builders {

/// Builds round roof. So far, supports only simple rectangle roofs.
class RoundRoofBuilder : public FlatRoofBuilder
{
    /// Specifies ridge direction (orientation).
    enum class Direction { Along, Across };
    /// Specifies amount of cirecle segments.
    const int RadialSegmentCount = 7;

public:
    RoundRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                     utymap::builders::MeshContext& meshContext) :
        FlatRoofBuilder(builderContext, meshContext), direction_()
    {
    }

    /// Sets roof direction. It should either be a string orientation (N, NNE, etc.) 
    /// or an angle in degree from north clockwise 
    void setDirection(const std::string& direction) override
    {
        direction_ = direction == "across" ? Direction::Across : Direction::Along;
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        if (!buildRound(polygon)) {
            FlatRoofBuilder::build(polygon);
            return;
        }

        builderContext_.meshBuilder
            .writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
    }

private:

    /// Tries to build round roof, returns false if polygon has unexpected shape.
    bool buildRound(utymap::meshing::Polygon& polygon) const
    {
        const auto range = polygon.outers[0];
        const auto size = range.second - range.first;
        if (!polygon.inners.empty() || polygon.outers.size() > 1 || size == 16)
            return false;

        // detect side with maximum length
        double maxSideLength = 0;
        auto maxSideIndex = range.first;
        const auto lastPointIndex = size - 2;
        for (std::size_t i = range.first; i < range.second; i += 2) {
            const auto nextIndex = i == lastPointIndex ? 0 : i + 2;

            utymap::meshing::Vector2 v0(polygon.points[i], polygon.points[i + 1]);
            utymap::meshing::Vector2 v1(polygon.points[nextIndex], polygon.points[nextIndex + 1]);

            auto length = utymap::meshing::Vector2::distance(v0, v1);
            if (length > maxSideLength) {
                maxSideIndex = i;
                maxSideLength = length;
            }
        }

        // specify points for sides and front/back
        utymap::meshing::Vector2 p0(polygon.points[maxSideIndex], polygon.points[maxSideIndex + 1]);
        auto next = nextIndex(maxSideIndex, lastPointIndex);
        utymap::meshing::Vector2 p1(polygon.points[next], polygon.points[next + 1]);
        next = nextIndex(next, lastPointIndex);
        utymap::meshing::Vector2 p2(polygon.points[next], polygon.points[next + 1]);
        next = nextIndex(next, lastPointIndex);
        utymap::meshing::Vector2 p3(polygon.points[next], polygon.points[next + 1]);

        // build round shape based on orientation
        if (direction_ == Direction::Along)
            buildRoundShape(polygon, p0, p1, p2, p3);
        else
            buildRoundShape(polygon, p1, p2, p3, p0);

        return true;
    }

    /// Builds round shape around front and back.  p1-p2 and p0-p3 are radial segments
    void buildRoundShape(const utymap::meshing::Polygon& polygon, const utymap::meshing::Vector2& p0,
        const utymap::meshing::Vector2& p1, const utymap::meshing::Vector2& p2, const utymap::meshing::Vector2& p3) const
    {
        // prepare pilar vectors
        const auto direction2d = (p3 - p0).normalized();
        const utymap::meshing::Vector3 direction(-direction2d.y, 0, direction2d.x);

        const utymap::meshing::Vector3 frontCenter((p0.x + p3.x) / 2, minHeight_, (p0.y + p3.y) / 2);
        const utymap::meshing::Vector3 backCenter((p1.x + p2.x) / 2, minHeight_, (p1.y + p2.y) / 2);
        
        const double frontRadius = utymap::meshing::Vector2::distance(utymap::meshing::Vector2(frontCenter.x, frontCenter.z), p0);
        const double backRadius = utymap::meshing::Vector2::distance(utymap::meshing::Vector2(backCenter.x, backCenter.z), p1);

        // these vectors we will rotation around direction vector
        const auto frontRotVector = (utymap::meshing::Vector3(p0.x, minHeight_, p0.y) - frontCenter).normalized();
        const auto backRotVector = (utymap::meshing::Vector3(p1.x, minHeight_, p1.y) - backCenter).normalized();

        // define uv mapping
        utymap::meshing::Vector2 u0(0, 0);

        const double angleStep = -pi / RadialSegmentCount;
        for (int j = 0; j < RadialSegmentCount; j++) {
            double firstAngle = j * angleStep;
            double secondAngle = (j + 1) * angleStep;

            auto firstAngleRotationFunc = utymap::utils::createRotationFunc(direction, firstAngle);
            auto secondAngleRotationFunc = utymap::utils::createRotationFunc(direction, secondAngle);

            // rotate vectors and move to original position
            const auto front1 = restore(firstAngleRotationFunc(frontRotVector), frontRadius, frontCenter);
            const auto front2 = restore(secondAngleRotationFunc(frontRotVector), frontRadius, frontCenter);
            const auto back1 = restore(firstAngleRotationFunc(backRotVector), backRadius, backCenter);
            const auto back2 = restore(secondAngleRotationFunc(backRotVector), backRadius, backCenter);

            addTriangle(frontCenter, front2, front1, u0, u0, u0);
            addTriangle(backCenter, back1, back2, u0, u0, u0);

            addTriangle(front1, back2, back1, u0, u0, u0);
            addTriangle(front2, back2, front1, u0, u0, u0);
        }
    }

    std::size_t nextIndex(std::size_t current, std::size_t max) const
    {
        return (current+=2) > max ? 0 : current;
    }

    utymap::meshing::Vector3 restore(const utymap::meshing::Vector3& normalized, double radius, utymap::meshing::Vector3 center) const
    {
        return utymap::meshing::Vector3(center.x + normalized.x * radius, minHeight_ + normalized.y * height_, center.z + normalized.z * radius);
    }

    Direction direction_;
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_ROUNDROOFBUILDER_HPP_DEFINED

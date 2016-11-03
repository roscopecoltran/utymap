#ifndef BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED

#include "builders/buildings/roofs/FlatRoofBuilder.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/Polygon.hpp"
#include "utils/CoreUtils.hpp"
#include "utils/GeometryUtils.hpp"
#include "utils/MathUtils.hpp"
#include "utils/MeshUtils.hpp"

#include <cmath>

namespace utymap { namespace builders {

/// Builds skillion roof. So far, supports only simple rectangle roofs.
class SkillionRoofBuilder : public FlatRoofBuilder
{
public:
    SkillionRoofBuilder(const utymap::builders::BuilderContext& builderContext,
                        utymap::builders::MeshContext& meshContext) :
        FlatRoofBuilder(builderContext, meshContext), direction_(0)
    {
    }
    
    /// Sets roof direction. It should either be a string orientation (N, NORTH, NNE, etc.) 
    ///  or an angle in degree from north clockwise 
    void setDirection(const std::string& direction) override
    {
        direction_ = utymap::utils::lexicalCast<double>(direction);
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        if (!buildSkillion(polygon)) {
            FlatRoofBuilder::build(polygon);
            return;
        }

        builderContext_.meshBuilder
            .writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
    }

private:

    /// Tries to build skillion roof
    /// NOTE So far we support only one simple polygon with holes when height is set.
    bool buildSkillion(utymap::meshing::Polygon& polygon) const
    {
        if (polygon.outers.size() == 1 && height_ > 0)
            return false;

        // get direction vector
        const auto grad = utymap::utils::deg2Rad(direction_);
        const auto direction = utymap::meshing::Vector2(std::sin(grad), std::cos(grad)).normalized();
        const auto maxHeight = minHeight_ + height_;

        // get center and points outside front/back from center in specified direction
        const auto& range = polygon.outers[0];
        const auto center = utymap::utils::getCentroid(polygon, range);
        const auto outBackPoint = center - direction * 0.1;
        const auto outFrontPoint = center + direction * 0.1;

        // copy geometry options and change some values to control mesh builder behaviour
        auto geometryOptions = meshContext_.geometryOptions;
        geometryOptions.heightOffset = minHeight_;
        geometryOptions.elevation = 0;
        geometryOptions.area = 0;

        // build mesh mostly to have triangulation in place
        utymap::meshing::Mesh mesh("");
        builderContext_.meshBuilder.addPolygon(mesh, polygon, geometryOptions, meshContext_.appearanceOptions);

        // detect front/back sides to set min/max elevation and get roof plane equation
        std::size_t frontSideIndex = mesh.vertices.size();
        std::size_t topBackSideIndex = mesh.vertices.size();
        double minDistance = std::numeric_limits<double>::max();
        double maxDistance = 0;
        const auto lastPointIndex = mesh.vertices.size() - 3;
        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            auto nextIndex = i == lastPointIndex ? 0 : i + 3;

            utymap::meshing::Vector2 v0(mesh.vertices[i], mesh.vertices[i + 1]);
            utymap::meshing::Vector2 v1(mesh.vertices[nextIndex], mesh.vertices[nextIndex + 1]);

            double r = utymap::utils::getIntersection(v0, v1, outBackPoint, outFrontPoint);
            if (r > std::numeric_limits<double>::lowest()) {
                const auto intersection = utymap::utils::getPointAlongLine(v0, v1, r);
                auto distance = utymap::meshing::Vector2::distance(outBackPoint, intersection);
                
                if (distance > maxDistance) { // Found new front face
                    frontSideIndex = i;
                    maxDistance = distance;
                } 
                if (distance < minDistance) { // Found new the highest point on back side
                    topBackSideIndex = (utymap::meshing::Vector2::distance(v0, intersection) <
                                        utymap::meshing::Vector2::distance(v1, intersection)) ? i : nextIndex;
                    minDistance = distance;
                }
            }
        }

        // fail to determine front/back: fallback to flat roof
        if (frontSideIndex > lastPointIndex || topBackSideIndex > lastPointIndex || frontSideIndex == topBackSideIndex)
            return false;

        // define points which are on top roof plane
        utymap::meshing::Vector3 p1(mesh.vertices[frontSideIndex], minHeight_, mesh.vertices[frontSideIndex + 1]);
        auto nextFrontSideIndex = frontSideIndex == lastPointIndex ? 0 : frontSideIndex + 3;
        utymap::meshing::Vector3 p2(mesh.vertices[nextFrontSideIndex], minHeight_, mesh.vertices[nextFrontSideIndex + 1]);
        utymap::meshing::Vector3 p3(mesh.vertices[topBackSideIndex], maxHeight, mesh.vertices[topBackSideIndex + 1]);

        // calculate equation of plane in classical form: Ax + By + Cz = d where n is (A, B, C)
        auto n = utymap::meshing::Vector3::cross(p1 - p2, p3 - p2);
        double d = n.x * p1.x + n.y * p1.y + n.z * p1.z;

        // loop over all vertices, calculate their height
        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            if (i == frontSideIndex || i == nextFrontSideIndex)
                continue;
            utymap::meshing::Vector2 p(mesh.vertices[i], mesh.vertices[i + 1]);
            mesh.vertices[i + 2] = utymap::utils::clamp(calcHeight(p, n, d), minHeight_, maxHeight);
        }

        // build faces
        double scale = utymap::utils::GeoUtils::getScaled(builderContext_.boundingBox,
                            meshContext_.appearanceOptions.textureScale, height_);
        utymap::meshing::Vector2 u0(0, 0);
        utymap::meshing::Vector2 u1(0, scale);
        utymap::meshing::Vector2 u2(scale, scale);
        utymap::meshing::Vector2 u3(scale, 0);

        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            if (i == frontSideIndex) 
                continue;

            auto nextIndex = i == lastPointIndex ? 0 : i + 3;

            utymap::meshing::Vector3 v0(mesh.vertices[i], minHeight_, mesh.vertices[i + 1]);
            utymap::meshing::Vector3 v1(mesh.vertices[i], mesh.vertices[i + 2], mesh.vertices[i + 1]);
            utymap::meshing::Vector3 v2(mesh.vertices[nextIndex], mesh.vertices[nextIndex + 2], mesh.vertices[nextIndex + 1]);
            utymap::meshing::Vector3 v3(mesh.vertices[nextIndex], minHeight_, mesh.vertices[nextIndex + 1]);

            if (i == nextFrontSideIndex) 
                addTriangle(v0, v2, v3, u0, u2, u3);
            else if (nextIndex == frontSideIndex) 
                addTriangle(v0, v1, v3, u0, u1, u3);
            else {
                addTriangle(v0, v2, v3, u0, u2, u3);
                addTriangle(v0, v1, v2, u0, u1, u2);
            }
        }

        utymap::utils::copyMesh(utymap::meshing::Vector3(0, 0, 0), mesh, meshContext_.mesh);

        return true;
    }

    /// Adds triangle to mesh from context.
    void addTriangle(const utymap::meshing::Vector3& v0, const utymap::meshing::Vector3& v1, const utymap::meshing::Vector3& v2, 
                     const utymap::meshing::Vector2& u0, const utymap::meshing::Vector2& u1, const utymap::meshing::Vector2& u2) const
    {
        builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v1, v2, u0, u1, u2,
            meshContext_.geometryOptions, meshContext_.appearanceOptions);
    }

    /// Calculates height of the 2d point using plane equation.
    static double calcHeight(const utymap::meshing::Vector2& p, const utymap::meshing::Vector3& n, double d)
    {
        return n.y != 0 ? (d - n.x * p.x - n.z * p.y) / n.y : 0;
    }

    double direction_;
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED

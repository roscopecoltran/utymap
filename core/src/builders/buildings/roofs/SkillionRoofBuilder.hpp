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
    //  or an angle in degree from north clockwise 
    void setDirection(const std::string& direction) override
    {
        direction_ = utymap::utils::lexicalCast<double>(direction);
    }

    void build(utymap::meshing::Polygon& polygon) override
    {
        if (!canBuild(polygon)) {
            // NOTE Fallback
            FlatRoofBuilder::build(polygon);
            return;
        }

        buildSkillion(polygon);

        builderContext_.meshBuilder
            .writeTextureMappingInfo(meshContext_.mesh, meshContext_.appearanceOptions);
    }

private:

    /// Checks whether we can build skillion roof for given polygon.
    /// NOTE So far we support only one simple polygon with holes when height is set.
    bool canBuild(const utymap::meshing::Polygon& polygon) const
    {
        return polygon.outers.size() == 1 && height_ > 0;
    }

    void buildSkillion(utymap::meshing::Polygon& polygon) const
    {
        // get direction vector
        const auto grad = utymap::utils::deg2Rad(direction_);
        const auto direction = utymap::meshing::Vector2(std::sin(grad), std::cos(grad)).normalized();

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

        //if (mesh.vertices.size() != 12)
        //    return;

        // detect front/back sides to set min/max elevation and get roof plane equation
        std::size_t frontSideIndex;
        std::size_t topBackSideIndex;
        double distanceToMostOuterFront = 0;
        double distanceToMostOuterBack = 0;
        const auto lastPointIndex = mesh.vertices.size() - 3;
        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            auto nextIndex = i == lastPointIndex ? 0 : i + 3;

            utymap::meshing::Vector2 v0(mesh.vertices[i], mesh.vertices[i + 1]);
            utymap::meshing::Vector2 v1(mesh.vertices[nextIndex], mesh.vertices[nextIndex + 1]);

            checkIntersection(v0, v1, center, outFrontPoint, true, i, frontSideIndex, distanceToMostOuterFront);
            checkIntersection(v0, v1, center, outBackPoint, false, i, topBackSideIndex, distanceToMostOuterBack);
        }

        // define points which are on top roof plane
        utymap::meshing::Vector3 p1(mesh.vertices[frontSideIndex], minHeight_, mesh.vertices[frontSideIndex + 1]);
        auto nextFrontSideIndex = frontSideIndex == lastPointIndex ? 0 : frontSideIndex + 3;
        utymap::meshing::Vector3 p2(mesh.vertices[nextFrontSideIndex], minHeight_, mesh.vertices[nextFrontSideIndex + 1]);
        topBackSideIndex = topBackSideIndex > lastPointIndex ? 0 : topBackSideIndex;
        utymap::meshing::Vector3 p3(mesh.vertices[topBackSideIndex], minHeight_ + height_, mesh.vertices[topBackSideIndex + 1]);

        // calculate equation of plane in classical form: Ax + By + Cz = d where n is (A, B, C)
        auto n = utymap::meshing::Vector3::cross(p1 - p2, p3 - p2);
        double d = n.x * p1.x + n.y * p1.y + n.z * p1.z;

        // loop over all vertices, calculate their height
        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            if (i == frontSideIndex || i == nextFrontSideIndex)
                continue;
            utymap::meshing::Vector2 p(mesh.vertices[i], mesh.vertices[i + 1]);
            mesh.vertices[i + 2] = calcHeight(p, n, d);
        }

        // build faces
        double scale = utymap::utils::GeoUtils::getScaled(builderContext_.boundingBox,
                                                          meshContext_.appearanceOptions.textureScale,
                                                          height_);
        for (std::size_t i = 0; i < mesh.vertices.size(); i += 3) {
            if (i == frontSideIndex)
                continue;

            auto nextIndex = i == lastPointIndex ? 0 : i + 3;

            utymap::meshing::Vector3 v0(mesh.vertices[i], minHeight_, mesh.vertices[i + 1]);
            utymap::meshing::Vector3 v1(mesh.vertices[i], mesh.vertices[i + 2], mesh.vertices[i + 1]);
            utymap::meshing::Vector3 v2(mesh.vertices[lastPointIndex], mesh.vertices[lastPointIndex + 2], mesh.vertices[lastPointIndex + 1]);
            utymap::meshing::Vector3 v3(mesh.vertices[lastPointIndex], minHeight_, mesh.vertices[lastPointIndex + 1]);

            if (i == nextFrontSideIndex) {
                builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v2, v3,
                    utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(scale, scale), utymap::meshing::Vector2(0, scale),
                    meshContext_.geometryOptions, meshContext_.appearanceOptions);
            }
            else if (nextIndex == frontSideIndex) {
                builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v1, v3,
                    utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(0, scale), utymap::meshing::Vector2(scale, scale),
                    meshContext_.geometryOptions, meshContext_.appearanceOptions);
            }
            else {
                builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v2, v3,
                    utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(scale, scale), utymap::meshing::Vector2(0, scale),
                    meshContext_.geometryOptions, meshContext_.appearanceOptions);

                builderContext_.meshBuilder.addTriangle(meshContext_.mesh, v0, v1, v2,
                    utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(0, scale), utymap::meshing::Vector2(scale, scale),
                    meshContext_.geometryOptions, meshContext_.appearanceOptions);
            }
        }

        utymap::utils::copyMesh(utymap::meshing::Vector3(0, 0, 0), mesh, meshContext_.mesh);
    }

    /// Checks intersection between two segments and changes distance and index if segment is new back/front side.
    static void checkIntersection(const utymap::meshing::Vector2 v0, const utymap::meshing::Vector2& v1,
                                  const utymap::meshing::Vector2& v3, const utymap::meshing::Vector2& v4,
                                  bool snapToStart, std::size_t currentIndex, std::size_t& maxIndex,
                                  double& maxDistance)
    {
        double r = utymap::utils::getIntersection(v0, v1, v3, v4);
        if (r > std::numeric_limits<double>::lowest()) {
            const auto intersection = utymap::utils::getPointAlongLine(v0, v1, r);
            auto distance = utymap::meshing::Vector2::distance(v3, intersection);

            if (distance < maxDistance)
                return;

            maxDistance = distance;
            maxIndex = snapToStart ||
                       (utymap::meshing::Vector2::distance(v0, intersection) <
                        utymap::meshing::Vector2::distance(v1, intersection))
                       ? currentIndex
                       : currentIndex + 3;
        }
    }

    /// Calculates height of the 2d point using plane equation.
    static double calcHeight(const utymap::meshing::Vector2& p, const utymap::meshing::Vector3& n, double d)
    {
        return n.y != 0
            ? (d - n.x * p.x - n.z * p.y) / n.y
            : 0; // degenerative case
    }

    //void buildSkillion(const utymap::meshing::Vector2& frontStart, const utymap::meshing::Vector2& frontEnd,
    //                   const utymap::meshing::Vector2& backStart, const utymap::meshing::Vector2& backEnd,
    //                   const utymap::meshing::Vector2& highestPoint) const
    //{
    //    double scale = utymap::utils::GeoUtils::getScaled(builderContext_.boundingBox,
    //                                                      meshContext_.appearanceOptions.textureScale,
    //                                                      height_);

    //    std::vector<double> heights;
    //    heights.reserve(4);




    //    // Specify all roof points
    //    utymap::meshing::Vector3 topBackStart(backStart.x, minHeight_ + height_, backStart.y);
    //    utymap::meshing::Vector3 topBackEnd(backEnd.x, minHeight_ + height_, backEnd.y);
    //    utymap::meshing::Vector3 bottomBackStart(backStart.x, minHeight_, backStart.y);
    //    utymap::meshing::Vector3 bottomBackEnd(backEnd.x, minHeight_, backEnd.y);

    //    utymap::meshing::Vector3 bottomFrontStart(frontStart.x, minHeight_, frontStart.y);
    //    utymap::meshing::Vector3 bottomFrontEnd(frontEnd.x, minHeight_, frontEnd.y);

    //    // Add triangles
    //    // top
    //    builderContext_.meshBuilder.addTriangle(meshContext_.mesh, topBackStart, bottomFrontEnd, bottomFrontStart,
    //        utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(0, scale), utymap::meshing::Vector2(scale, scale),
    //        meshContext_.geometryOptions, meshContext_.appearanceOptions);

    //    builderContext_.meshBuilder.addTriangle(meshContext_.mesh, topBackStart, bottomFrontStart, topBackEnd,
    //        utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(scale, scale), utymap::meshing::Vector2(0, scale),
    //        meshContext_.geometryOptions, meshContext_.appearanceOptions);
    //           
    //    // left
    //    builderContext_.meshBuilder.addTriangle(meshContext_.mesh, bottomFrontStart, bottomBackEnd, topBackEnd,
    //        utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(0, scale), utymap::meshing::Vector2(scale, scale),
    //        meshContext_.geometryOptions, meshContext_.appearanceOptions);

    //    // right
    //    builderContext_.meshBuilder.addTriangle(meshContext_.mesh, bottomBackStart, bottomFrontEnd, topBackStart,
    //        utymap::meshing::Vector2(scale, 0), utymap::meshing::Vector2(0, 0), utymap::meshing::Vector2(scale, scale),
    //        meshContext_.geometryOptions, meshContext_.appearanceOptions);

    //    meshContext_.geometryOptions.heightOffset = height_;
    //    // back
    //    builderContext_.meshBuilder.addPlane(meshContext_.mesh, backStart, backEnd, minHeight_, minHeight_,
    //        meshContext_.geometryOptions, meshContext_.appearanceOptions);
    //}

    double direction_;
};

}}

#endif // BUILDERS_BUILDINGS_ROOFS_SKILLIONROOFBUILDER_HPP_DEFINED

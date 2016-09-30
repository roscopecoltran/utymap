#ifndef BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace utymap { namespace builders {

/// Builds icosphere.
/// See http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
class IcoSphereGenerator final : public AbstractGenerator
{
    /// Helper class for calculations
    struct TriangleIndices
    {
        std::size_t V1, V2, V3;
        TriangleIndices(std::size_t v1, std::size_t v2, std::size_t v3) :
                V1(v1), V2(v2), V3(v3)
        {
        }
    };
public:

    IcoSphereGenerator(const utymap::builders::BuilderContext& builderContext,
                       utymap::builders::MeshContext& meshContext):
            AbstractGenerator(builderContext, meshContext),
            center_(), radius_(0), height_(0), recursionLevel_(0), isSemiSphere_(false)
    {
    }

    /// Sets center of icosphere.
    IcoSphereGenerator& setCenter(const utymap::meshing::Vector3& center)
    {
        center_ = center;
        return *this;
    }

    /// Sets radius of icosphere.
    IcoSphereGenerator& setRadius(double radius, double height = 1)
    {
        radius_ = radius;
        height_ = height;
        return *this;
    }

    /// Sets recursion level.
    IcoSphereGenerator& setRecursionLevel(int recursionLevel)
    {
        recursionLevel_ = recursionLevel;
        return *this;
    }

    /// If true than only half will be generated.
    IcoSphereGenerator& isSemiSphere(bool value)
    {
        isSemiSphere_ = value;
        return *this;
    }

    void generate() override
    {
        // create 12 vertices of a icosahedron
        double t = (1 + std::sqrt(5)) / 2;

        vertexList_.push_back(utymap::meshing::Vector3(-1, t, 0).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(1, t, 0).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(-1, -t, 0).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(1, -t, 0).normalized());

        vertexList_.push_back(utymap::meshing::Vector3(0, -1, t).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(0, 1, t).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(0., -1, -t).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(0, 1, -t).normalized());

        vertexList_.push_back(utymap::meshing::Vector3(t, 0, -1).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(t, 0, 1).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(-t, 0, -1).normalized());
        vertexList_.push_back(utymap::meshing::Vector3(-t, 0, 1).normalized());

        // create 20 triangles of the icosahedron
        std::vector<TriangleIndices> faces;
        // 5 faces around point 0
        faces.push_back(TriangleIndices(0, 11, 5));
        faces.push_back(TriangleIndices(0, 5, 1));
        faces.push_back(TriangleIndices(0, 1, 7));
        faces.push_back(TriangleIndices(0, 7, 10));
        faces.push_back(TriangleIndices(0, 10, 11));

        // 5 adjacent faces
        faces.push_back(TriangleIndices(1, 5, 9));
        faces.push_back(TriangleIndices(5, 11, 4));
        if (!isSemiSphere_)
            faces.push_back(TriangleIndices(11, 10, 2));

        faces.push_back(TriangleIndices(10, 7, 6));
        faces.push_back(TriangleIndices(7, 1, 8));

        // 5 faces around point 3
        if (!isSemiSphere_) {
            faces.push_back(TriangleIndices(3, 9, 4));
            faces.push_back(TriangleIndices(3, 4, 2));
            faces.push_back(TriangleIndices(3, 2, 6));
            faces.push_back(TriangleIndices(3, 6, 8));
            faces.push_back(TriangleIndices(3, 8, 9));
        }

        // 5 adjacent faces
        faces.push_back(TriangleIndices(4, 9, 5));
        if (!isSemiSphere_) {
            faces.push_back(TriangleIndices(2, 4, 11));
            faces.push_back(TriangleIndices(6, 2, 10));
        }
        faces.push_back(TriangleIndices(8, 6, 7));
        faces.push_back(TriangleIndices(9, 8, 1));

        // refine triangles
        for (int i = 0; i < recursionLevel_; i++) {
            std::vector<TriangleIndices> faces2;
            for (const auto& tri : faces) {
                // replace triangle by 4 triangles
                auto a = getMiddlePoint(tri.V1, tri.V2);
                auto b = getMiddlePoint(tri.V2, tri.V3);
                auto c = getMiddlePoint(tri.V3, tri.V1);

                faces2.push_back(TriangleIndices(tri.V1, a, c));
                faces2.push_back(TriangleIndices(tri.V2, b, a));
                faces2.push_back(TriangleIndices(tri.V3, c, b));
                faces2.push_back(TriangleIndices(a, b, c));
            }
            faces = faces2;
        }
        generateMesh(faces);

        // clear state to allow reusage
        middlePointIndexCache_.clear();
        vertexList_.clear();
    }

private:
    ///  Returns index of point in the middle of p1 and p2.
    std::size_t getMiddlePoint(std::size_t p1, std::size_t p2)
    {
        // first check if we have it already
        bool firstIsSmaller = p1 < p2;
        std::uint64_t smallerIndex = firstIsSmaller ? p1 : p2;
        std::uint64_t greaterIndex = firstIsSmaller ? p2 : p1;
        std::uint64_t key = (smallerIndex << 32) + greaterIndex;

        auto ret = middlePointIndexCache_.find(key);
        if (ret != middlePointIndexCache_.end())
            return ret->second;

        // not in cache, calculate it
        utymap::meshing::Vector3 point1 = vertexList_[p1];
        utymap::meshing::Vector3 point2 = vertexList_[p2];
        utymap::meshing::Vector3 middle
        (
            (point1.x + point2.x) / 2,
            (point1.y + point2.y) / 2,
            (point1.z + point2.z) / 2
        );

        // add vertex makes sure point is on unit sphere
        std::size_t size = vertexList_.size();
        vertexList_.push_back(middle.normalized());

        // store it, return index
        middlePointIndexCache_.insert(std::make_pair(key, size));

        return size;
    }

    void generateMesh(const std::vector<TriangleIndices>& faces)
    {
        for (auto i = 0; i < faces.size(); ++i) {
            auto face = faces[i];
            addTriangle(
                scale(vertexList_[face.V1]) + center_,
                scale(vertexList_[face.V2]) + center_,
                scale(vertexList_[face.V3]) + center_);

        }
    }

    utymap::meshing::Vector3 scale(const utymap::meshing::Vector3& v) const
    {
        return utymap::meshing::Vector3(
            v.x * radius_ * 1.5,
            v.y * height_,
            v.z * radius_);
    }

utymap::meshing::Vector3 center_;
double radius_, height_;
int recursionLevel_;
bool isSemiSphere_;

std::unordered_map<std::uint64_t, std::size_t> middlePointIndexCache_;
std::vector<utymap::meshing::Vector3> vertexList_;

};

}}

#endif // BUILDERS_GENERATORS_ICOSPHEREGENERATOR_HPP_DEFINED

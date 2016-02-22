#ifndef SCENE_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED
#define SCENE_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "mapcss/ColorGradient.hpp"

namespace utymap { namespace scene {

// Responsible for building facade wall in low poly quality.
class LowPolyWallBuilder
{
protected:
    // Represents point in 3d where y points up.
    struct Point3d
    {
        double x, y, z;
        Point3d(double x, double y, double z) : x(x), y(y), z(z) {}
    };

public:
    LowPolyWallBuilder(const utymap::meshing::Mesh& mesh,
                       const utymap::mapcss::ColorGradient& gradient)
        : mesh_(mesh), gradient_(gradient), height_(12), minHeight_(0)
    {
    }

    // Sets height of wall.
    inline LowPolyWallBuilder& setHeight(double height) { height_ = height; return *this; }

    // Sets height above ground level.
    inline LowPolyWallBuilder& setMinHeight(double minHeight) { minHeight_ = minHeight; return *this; }

    void build(const utymap::GeoCoordinate& start, const utymap::GeoCoordinate& end)
    {
        addPlane(start, end);
    }

private:

    void addPlane(const utymap::GeoCoordinate& p1, const utymap::GeoCoordinate& p2)
    {
        double top = minHeight_ + height_;
        int color = 0, index = mesh_.triangles.size();
        // first triangle
        addVertex(p1, minHeight_, index++, color);
        addVertex(p2, minHeight_, index++, color);
        addVertex(p2, top, index++, color);

        // second triangle
        addVertex(p2, minHeight_, index++, color);
        addVertex(p2, top, index++, color);
        addVertex(p1, top, index, color);
    }

    inline void addVertex(const utymap::GeoCoordinate& coordinate, double height, int triIndex, int color)
    {
        mesh_.vertices.push_back(coordinate.longitude);
        mesh_.vertices.push_back(coordinate.latitude);
        mesh_.vertices.push_back(height);
        mesh_.triangles.push_back(triIndex);
        mesh_.colors.push_back(color);
    }

    const utymap::meshing::Mesh& mesh_;
    const utymap::mapcss::ColorGradient& gradient_;
    double height_, minHeight_;
};

}}

#endif // SCENE_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

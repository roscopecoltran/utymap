#ifndef BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED
#define BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "meshing/MeshTypes.hpp"
#include "mapcss/ColorGradient.hpp"
#include "utils/GradientUtils.hpp"

namespace utymap { namespace builders {

// Responsible for building facade wall in low poly quality.
class LowPolyWallBuilder
{
public:
    LowPolyWallBuilder(utymap::meshing::Mesh& mesh,
                       const utymap::mapcss::ColorGradient& gradient)
         : mesh_(mesh), gradient_(gradient), height_(12), minHeight_(0), colorNoiseFreq_(0)
    {
    }

    // Sets height of wall.
    inline LowPolyWallBuilder& setHeight(double height) { height_ = height; return *this; }

    // Sets height above ground level.
    inline LowPolyWallBuilder& setMinHeight(double minHeight) { minHeight_ = minHeight; return *this; }

    // Sets color freq.
    inline LowPolyWallBuilder& setColorNoise(double freq) { colorNoiseFreq_ = freq; return *this; }

    void build(const utymap::GeoCoordinate& start, const utymap::GeoCoordinate& end)
    {
        addPlane(start, end);
    }

private:

    void addPlane(const utymap::GeoCoordinate& p1, const utymap::GeoCoordinate& p2)
    {
        double top = minHeight_ + height_;
        auto index = mesh_.triangles.size();
        int color = utymap::utils::GradientUtils
            ::getColor(gradient_, p1.longitude, p1.latitude, height_, colorNoiseFreq_);

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

    utymap::meshing::Mesh& mesh_;
    const utymap::mapcss::ColorGradient& gradient_;
    double height_, minHeight_, colorNoiseFreq_;
};

}}

#endif // BUILDERS_BUILDINGS_FACADES_LOWPOLYWALLBUILDER_HPP_DEFINED

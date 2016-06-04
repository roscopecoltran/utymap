#ifndef BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "utils/MathUtils.hpp"

#include <cmath>

namespace utymap { namespace builders {

// Generates cylinder.
class CylinderGenerator : public AbstractGenerator
{
public:

    CylinderGenerator(const utymap::builders::BuilderContext& builderContext,
                      utymap::builders::MeshContext& meshContext) :
            AbstractGenerator(builderContext, meshContext),
            center_(), radius_(0), height_(0), radialSegments_(0), maxSegmentHeight_(0)
    {
    }

    // Sets center of cylinder.
    CylinderGenerator& setCenter(const utymap::meshing::Vector3& center)
    {
        center_ = center;
        return *this;
    }

    // Sets radius of cylinder.
    CylinderGenerator& setRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    // Sets radius of cylinder.
    CylinderGenerator& setHeight(double height)
    {
        height_ = height;
        return *this;
    }

    CylinderGenerator& setRadialSegments(int radialSegments)
    {
        radialSegments_ = radialSegments;
        return *this;
    }

    CylinderGenerator& setMaxSegmentHeight(double maxSegmentHeight)
    {
        maxSegmentHeight_ = maxSegmentHeight;
        return *this;
    }

    void generate()
    {
        int heightSegments = (int) std::ceil(height_ / maxSegmentHeight_);

        double heightStep = height_ / heightSegments;
        double angleStep = 2 * pi / radialSegments_;

        for (int j = 0; j < radialSegments_; j++) {
            double firstAngle = j * angleStep;
            double secondAngle = (j == radialSegments_ - 1 ? 0 : j + 1) * angleStep;

            auto first = utymap::meshing::Vector2(
                    radius_ * std::cos(firstAngle) + center_.x,
                    radius_ * std::sin(firstAngle) + center_.z);

            auto second = utymap::meshing::Vector2(
                    radius_ * std::cos(secondAngle) + center_.x,
                    radius_ * std::sin(secondAngle) + center_.z);

            // bottom cap
            addTriangle(center_,
                        utymap::meshing::Vector3(first.x, center_.y, first.y),
                        utymap::meshing::Vector3(second.x, center_.y, second.y));

            // top cap
            addTriangle(utymap::meshing::Vector3(center_.x, center_.y + height_, center_.z),
                        utymap::meshing::Vector3(second.x, center_.y + height_, second.y),
                        utymap::meshing::Vector3(first.x, center_.y + height_, first.y));

            for (int i = 0; i < heightSegments; i++) {
                double bottomHeight = i * heightStep + center_.y;
                double topHeight = (i + 1) * heightStep + center_.y;

                utymap::meshing::Vector3 v0(first.x, bottomHeight, first.y);
                utymap::meshing::Vector3 v1(second.x, bottomHeight, second.y);
                utymap::meshing::Vector3 v2(second.x, topHeight, second.y);
                utymap::meshing::Vector3 v3(first.x, topHeight, first.y);

                addTriangle(v0, v2, v1);
                addTriangle(v3, v2, v0);
            }
        }
    }

private:

    utymap::meshing::Vector3 center_;
    int radialSegments_;
    double radius_, height_, maxSegmentHeight_;

};
}}

#endif // BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED

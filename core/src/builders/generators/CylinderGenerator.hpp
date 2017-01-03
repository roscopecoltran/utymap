#ifndef BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "math/Quaternion.hpp"
#include "utils/MathUtils.hpp"

namespace utymap { namespace builders {

/// Generates cylinder.
class CylinderGenerator : public AbstractGenerator
{
public:

    CylinderGenerator(const utymap::builders::BuilderContext& builderContext,
        utymap::builders::MeshContext& meshContext) :
        AbstractGenerator(builderContext, meshContext)
    {
    }

    /// Sets center of cylinder.
    CylinderGenerator& setCenter(const utymap::math::Vector3& center)
    {
        center_ = center;
        return *this;
    }

    /// Sets radius of cylinder.
    CylinderGenerator& setRadius(const utymap::math::Vector3& radius)
    {
        radius1_ = radius;
        radius2_ = radius;
        return *this;
    }

    /// Sets radius of cylinder for first and last segments.
    CylinderGenerator& setRadius(const utymap::math::Vector3& radius1,
                                 const utymap::math::Vector3& radius2)
    {
        radius1_ = radius1;
        radius2_ = radius2;
        return *this;
    }

    /// Sets orientation of cylinder in 3D space.
    CylinderGenerator& SetDirection(const utymap::math::Vector3& direction,
                                    const utymap::math::Vector3& right)
    {
        direction_ = direction;
        right_ = right;
        return *this;
    }

    /// Sets radius of cylinder.
    CylinderGenerator& setHeight(double height)
    {
        height_ = height;
        return *this;
    }

    /// Sets radial segment count.
    CylinderGenerator& setRadialSegments(int radialSegments)
    {
        radialSegments_ = radialSegments;
        return *this;
    }

    /// Sets max segment height.
    CylinderGenerator& setMaxSegmentHeight(double maxSegmentHeight)
    {
        maxSegmentHeight_ = maxSegmentHeight;
        return *this;
    }

    void generate() override
    {
        int heightSegments = static_cast<int>(std::ceil(height_ / maxSegmentHeight_));
        double heightStep = height_ / heightSegments;
        double angleStep = 2 * pi / radialSegments_;

        for (int j = 0; j < radialSegments_; ++j) {
            auto direction1 = utymap::math::Quaternion::fromAngleAxis(j * angleStep, direction_);
            auto direction2 = utymap::math::Quaternion::fromAngleAxis((j == radialSegments_ - 1 ? 0 : j + 1) * angleStep, direction_);

            for (int i = 0; i < heightSegments; i++) {
                auto radius1 = getRadius(static_cast<double>(heightSegments - i) / heightSegments);
                auto radius2 = getRadius(static_cast<double>(heightSegments - i - 1) / heightSegments);

                auto center1 = center_ + direction_ * (heightStep * i);
                auto center2 = center_ + direction_ * (heightStep * (i + 1));

                auto v0 = center1 + scale(direction1 * right_, radius1);
                auto v1 = center1 + scale(direction2 * right_, radius1);
                auto v2 = center2 + scale(direction2 * right_, radius2);
                auto v3 = center2 + scale(direction1 * right_, radius2);

                // add side.
                addTriangle(v1, v2, v0);
                addTriangle(v0, v2, v3);

                // add bottom cap part.
                if (i == 0)
                    addTriangle(center1, v1, v0);
                // add top cap part.
                if (i == heightSegments - 1 && !isCone())
                    addTriangle(center2, v3, v2);
            }
        }

        builderContext_.meshBuilder.writeTextureMappingInfo(meshContext_.mesh,
            meshContext_.appearanceOptions);
    }

private:

    bool isCone() const
    {
        return radius2_ == utymap::math::Vector3::zero();
    }

    utymap::math::Vector3 getRadius(double value) const
    {
        return utymap::math::Vector3(utymap::utils::lerp(radius2_.x, radius1_.x, value),
                                     utymap::utils::lerp(radius2_.y, radius1_.y, value),
                                     utymap::utils::lerp(radius2_.z, radius1_.z, value));
    }

    static utymap::math::Vector3 scale(const utymap::math::Vector3& value, const utymap::math::Vector3& size)
    {
        return utymap::math::Vector3(value.x * size.x, value.y * size.y, value.z * size.z);
    }

    utymap::math::Vector3 center_ = utymap::math::Vector3::zero();
    utymap::math::Vector3 direction_ = utymap::math::Vector3::up();
    utymap::math::Vector3 right_ = utymap::math::Vector3::right();
    utymap::math::Vector3 radius1_ = utymap::math::Vector3::zero();
    utymap::math::Vector3 radius2_ = utymap::math::Vector3::zero();
    double height_ = 0, maxSegmentHeight_ = 0;
    int radialSegments_ = 0;

};

}}
#endif // BUILDERS_GENERATORS_CYLINDERGENERATOR_HPP_DEFINED

#ifndef BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "builders/generators/AbstractGenerator.hpp"
#include "utils/GeoUtils.hpp"

#include <algorithm>
#include <vector>

namespace utymap { namespace builders {

/// Provides the way to generate wall.
class WallGenerator : public AbstractGenerator
{
public:
    WallGenerator(const utymap::builders::BuilderContext& builderContext,
                  utymap::builders::MeshContext& meshContext) :
        AbstractGenerator(builderContext, meshContext),
        geometry_(), width_(0), height_(0), length_(0)
    {
    }

    WallGenerator& setGeometry(const std::vector<utymap::GeoCoordinate>& geometry)
    {
        geometry_ = geometry;
        return *this;
    }

    WallGenerator& setWidth(double width)
    {
        width_ = width;
        return *this;
    }

    WallGenerator& setHeight(double height)
    {
        height_ = height;
        return *this;
    }

    WallGenerator& setLength(double length)
    {
        length_ = length;
        return *this;
    }

    void generate() override
    {
        auto size = geometry_.size();

        for (std::size_t i = 0; i < size - 1; ++i) {
            const auto& p0 = geometry_[i];
            const auto& p1 = geometry_[i + 1];

            double distanceInMeters = utymap::utils::GeoUtils::distance(p0, p1);
            int count = std::max(static_cast<int>(distanceInMeters / length_), 1);

            auto start = p0;
            for (int j = 1; j <= count; ++j) {
                double offset = static_cast<double>(j) / count;
                auto end = count == 1 ? p1 : utymap::utils::GeoUtils::newPoint(p0, p1, offset);
                buildSegment(utymap::math::Vector2(start.longitude, start.latitude),
                             utymap::math::Vector2(end.longitude, end.latitude));
                start = end;
            }
        }
    }

private:

    /// Builds segment separately. NOTE better to connect them.
    void buildSegment(const utymap::math::Vector2& p0, const utymap::math::Vector2& p1)
    {
        // get segments.
        auto direction = (p1 - p0).normalized();
        utymap::math::Vector2 normal(-direction.y, direction.x);
        auto leftP0 = p0 + normal * width_;
        auto leftP1 = p1 + normal * width_;
        auto rightP0 = p0 - normal * width_;
        auto rightP1 = p1 - normal * width_;

        auto bottom = builderContext_.eleProvider.getElevation(builderContext_.quadKey, p0.y, p0.x);
        auto top = bottom + height_;

        addPlane(leftP1, leftP0, bottom, top);
        addPlane(rightP1, leftP1, bottom, top);
        addPlane(rightP0, rightP1, bottom, top);
        addPlane(leftP0, rightP0, bottom, top);

        addTriangle(utymap::math::Vector3(leftP0.x, top, leftP0.y),
                    utymap::math::Vector3(leftP1.x, top, leftP1.y),
                    utymap::math::Vector3(rightP0.x, top, rightP0.y));
        
        addTriangle(utymap::math::Vector3(rightP0.x, top, rightP0.y),
                    utymap::math::Vector3(leftP1.x, top, leftP1.y), 
                    utymap::math::Vector3(rightP1.x, top, rightP1.y));
    }

    std::vector<utymap::GeoCoordinate> geometry_;
    double width_, height_, length_;
};

}}

#endif // BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED

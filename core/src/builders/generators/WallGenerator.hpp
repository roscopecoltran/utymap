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

    typedef std::vector<utymap::GeoCoordinate>::const_iterator Iterator;

    WallGenerator(const utymap::builders::BuilderContext& builderContext,
                  utymap::builders::MeshContext& meshContext) :
        AbstractGenerator(builderContext, meshContext),
        begin_(), end_(), width_(0), height_(0), length_(0), offset_(0)
    {
    }

    WallGenerator& setGeometry(Iterator begin, Iterator end)
    {
        begin_ = begin;
        end_ = end;
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

    WallGenerator& setOffset(double offset)
    {
        offset_ = offset;
        return *this;
    }

    void generate() override
    {
        if (begin_ == end_) return;

        auto size = static_cast<std::size_t>(std::distance(begin_, end_) - 1);
        auto fullLength = length_ + offset_;
        auto ratio = length_ / fullLength;
        for (std::size_t i = 0; i < size; ++i) {
            const auto& p0 = *(begin_ + i);
            const auto& p1 = *(begin_ + i + 1);

            double distanceInMeters = utymap::utils::GeoUtils::distance(p0, p1);
            int count = std::max(static_cast<int>(distanceInMeters / fullLength), 1);

            auto start = p0;
            for (int j = 1; j <= count; ++j) {
                double offset = static_cast<double>(j) / count;
                auto end = count == 1 ? p1 : utymap::utils::GeoUtils::newPoint(p0, p1, offset);
                buildSegment(utymap::math::Vector2(start.longitude, start.latitude),
                    utymap::math::Vector2(start.longitude + (end.longitude - start.longitude) * ratio,
                                          start.latitude + (end.latitude - start.latitude) * ratio));
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

    Iterator begin_, end_;
    double width_, height_, length_, offset_;
};

}}

#endif // BUILDERS_GENERATORS_WALLGENERATOR_HPP_DEFINED

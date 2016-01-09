#ifndef INDEX_LODRANGE_HPP_DEFINED
#define INDEX_LODRANGE_HPP_DEFINED

#include "index/GeoUtils.hpp"

#include <string>

namespace utymap { namespace index {

// Represents level of details range.
struct LodRange
{
    int start;
    int end;

    LodRange(int s, int e) :
        start(s),
        end(e)
    {
        int min = utymap::index::GeoUtils::MinLevelOfDetails;
        int max = utymap::index::GeoUtils::MaxLevelOfDetails;

        if (s < min || s > max || e < min || e > max)
            throw std::domain_error("Unsupported level of details");
    }
};

}}

#endif // INDEX_LODRANGE_HPP_DEFINED

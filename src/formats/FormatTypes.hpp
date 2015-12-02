#ifndef FORMATS_TYPES_HPP_DEFINED
#define FORMATS_TYPES_HPP_DEFINED

#include "GeoCoordinate.hpp"

#include <vector>
#include <cstdint>
#include <string>

namespace utymap { namespace formats {

enum FormatType
{
    Pbf,
    Xml,
    Shape
};

struct Tag
{
    std::string key;
    std::string value;
};

// this type is specific for osm formats
struct RelationMember
{
    uint64_t refId;
    std::string type;
    std::string role;
};

// this type is specific for shapefile format
struct PolygonMember
{
    bool isRing;
    std::vector<GeoCoordinate> coordinates;
};

typedef std::vector<Tag> Tags;
typedef std::vector<RelationMember> RelationMembers;
typedef std::vector<PolygonMember> PolygonMembers;

}}

#endif // FORMATS_TYPES_HPP_DEFINED

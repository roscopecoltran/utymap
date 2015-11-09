#ifndef TYPES_HPP_DEFINED
#define TYPES_HPP_DEFINED

#include "stdafx.h"

#include <cstdint>
#include <string>

struct BoundingBox
{
    double minLatitude;
    double minLongitude;
    double maxLatitude;
    double maxLongitude;
};

struct Tag
{
    std::string key;
    std::string value;
};

struct RelationMember
{
    uint64_t refId;
    std::string type;
    std::string role;
};

typedef std::vector<Tag> TagCollection;
typedef std::vector<RelationMember> RelationMembers;

#endif // TYPES_HPP_DEFINED

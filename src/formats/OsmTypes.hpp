#ifndef FORMATS_TYPES_HPP_DEFINED
#define FORMATS_TYPES_HPP_DEFINED

#include <vector>
#include <cstdint>
#include <string>

namespace utymap { namespace formats {

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

}}

#endif // FORMATS_TYPES_HPP_DEFINED

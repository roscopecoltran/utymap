#ifndef ENTITIES_ELEMENT_HPP_DEFINED
#define ENTITIES_ELEMENT_HPP_DEFINED

#include "ElementVisitor.hpp"
#include "../index/StringTable.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace utymap { namespace entities {

// Represents tag
struct Tag
{
    // Returns key string id.
    uint32_t key;
    // Returns value string id.
    uint32_t value;
};

// Represents element stored in index.
class Element
{
public:
    typedef std::vector<Tag> TagCollection;

    // Returns id of given element.
    uint64_t id;
    // Returns tag collection represented by vector of tuple<uint,uint>.
    TagCollection tags;

    // Visitor method.
    virtual void accept(ElementVisitor&) const = 0;

    // Returns string reprsentation of element.
    virtual std::string toString(utymap::index::StringTable& stringTable)
    {
        std::string result;
        result.reserve(64);

        result = '[' + std::to_string(id) + ']' + "{";
        for (Tag& tag : tags) {
            result += (stringTable.getString(tag.key) + ':' +
                stringTable.getString(tag.value) + ',');
        }
        result += "}";

        return result;
    }
};

}}
#endif // ENTITIES_ELEMENT_HPP_DEFINED

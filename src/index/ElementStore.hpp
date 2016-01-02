#ifndef INDEX_ELEMENTSTORE_HPP_DEFINED
#define INDEX_ELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Defines API to store elements.
class ElementStore
{
public:
    // Stores element in storage.
    virtual void store(const QuadKey& quadKey, const utymap::entities::Element& element) = 0;
};

}}

#endif // INDEX_ELEMENTSTORE_HPP_DEFINED

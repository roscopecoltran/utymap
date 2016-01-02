#ifndef INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED
#define INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"
#include "index/StringTable.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in memory.
class InMemoryElementStore : public ElementStore
{
public:
    InMemoryElementStore(StringTable& stringTable);

    ~InMemoryElementStore();

    // Stores element in storage.
    void store(const QuadKey& quadKey, const utymap::entities::Element& element);

private:
    class InMemoryElementStoreImpl;
    std::unique_ptr<InMemoryElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

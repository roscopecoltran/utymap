#ifndef INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED
#define INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"
#include "mapcss/StyleProvider.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in memory.
class InMemoryElementStore : public ElementStore
{
public:
    InMemoryElementStore(const utymap::mapcss::StyleProvider& styleProvider,
                         utymap::index::StringTable& stringTable);

    ~InMemoryElementStore();

protected:
    void searchImpl(const utymap::QuadKey& quadKey);
    void storeImpl(const utymap::entities::Element& element, const utymap::QuadKey& quadKey);

private:
    class InMemoryElementStoreImpl;
    std::unique_ptr<InMemoryElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

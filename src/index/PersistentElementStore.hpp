#ifndef INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED
#define INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"
#include "mapcss/StyleProvider.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in persistent store.
class PersistentElementStore : public ElementStore
{
public:
    PersistentElementStore(const std::string& path,
                           const utymap::mapcss::StyleProvider& styleProvider,
                           utymap::index::StringTable& stringTable);

    ~PersistentElementStore();

protected:
    void searchImpl(const utymap::QuadKey& quadKey);
    void storeImpl(const utymap::entities::Element& element, const utymap::QuadKey& quadKey);

private:
    class PersistentElementStoreImpl;
    std::unique_ptr<PersistentElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED

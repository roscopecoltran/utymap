#ifndef INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED
#define INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"
#include "index/StringTable.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in persistent store.
class PersistentElementStore : public ElementStore
{
public:
    PersistentElementStore(const std::string& path, StringTable& stringTable);

    ~PersistentElementStore();

    // Stores element in storage.
    void store(const QuadKey& quadKey, const utymap::entities::Element& element);

private:
    class PersistentElementStoreImpl;
    std::unique_ptr<PersistentElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_PERSISTENTELEMENTSTORE_HPP_DEFINED

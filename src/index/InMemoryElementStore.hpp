#ifndef INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED
#define INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

#include "QuadKey.hpp"
#include "entities/Element.hpp"
#include "index/ElementStore.hpp"

#include <string>
#include <memory>

namespace utymap { namespace index {

// Provides API to store elements in memory.
class InMemoryElementStore : public ElementStore
{
public:
    InMemoryElementStore(const StyleFilter& styleFilter);

    ~InMemoryElementStore();

protected:
    const StyleFilter& getStyleFilter() const;
    void store(const utymap::entities::Element& element, const QuadKey& quadKey);

private:
    class InMemoryElementStoreImpl;
    std::unique_ptr<InMemoryElementStoreImpl> pimpl_;
};

}}

#endif // INDEX_INMEMORYELEMENTSTORE_HPP_DEFINED

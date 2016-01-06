#ifndef INDEX_STYLEPROVIDER_HPP_DEFINED
#define INDEX_STYLEPROVIDER_HPP_DEFINED

#include "index/StringTable.hpp"
#include "entities/Element.hpp"
#include "mapcss/Stylesheet.hpp"
#include "mapcss/Style.hpp"

#include <memory>

namespace utymap { namespace mapcss {

// This class responsible for filtering elements.
class StyleProvider
{
public:

    StyleProvider(const utymap::mapcss::StyleSheet&, utymap::index::StringTable&);

    ~StyleProvider();

     utymap::mapcss::Style get(const utymap::entities::Element&, int levelOfDetails) const;

private:
    class StyleProviderImpl;
    std::unique_ptr<StyleProviderImpl> pimpl_;
};

}}

#endif // INDEX_STYLEPROVIDER_HPP_DEFINED

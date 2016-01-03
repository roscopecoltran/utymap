#ifndef INDEX_STYLEFILTER_HPP_DEFINED
#define INDEX_STYLEFILTER_HPP_DEFINED

#include "StringTable.hpp"
#include "entities/Element.hpp"
#include "mapcss/Stylesheet.hpp"

#include <memory>

namespace utymap { namespace index {

// This class responsible for filtering elements.
class StyleFilter
{
public:
    StyleFilter(const utymap::mapcss::StyleSheet&,
                      utymap::index::StringTable&);

    ~StyleFilter();

    //  Returns true if element is defined in stylesheet..
    inline bool isApplicable(const utymap::entities::Element&, int levelOfDetails) const;

private:
    class StyleFilterImpl;
    std::unique_ptr<StyleFilterImpl> pimpl_;
};

}}

#endif // INDEX_STYLEFILTER_HPP_DEFINED

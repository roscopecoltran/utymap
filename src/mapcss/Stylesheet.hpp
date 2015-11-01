#ifndef MAPCSS_STYLESHEET_HPP_DEFINED
#define MAPCSS_STYLESHEET_HPP_DEFINED

#include <string>

namespace utymap { namespace mapcss {


struct SimpleSelector
{
public:

    void setElementName(const std::string& name) 
    {
        elementName_ = name;
    }

    std::string elementName_;
};

struct Selector
{
public:

    void addSimpleSelector(const SimpleSelector& s)
    {
        simpleSelectors_.push_back(s);
    }

    std::vector<SimpleSelector> simpleSelectors_;
};

struct StyleSheet
{

};

}} // ns end utymap::mapcss

#endif // MAPCSS_STYLESHEET_HPP_DEFINED
